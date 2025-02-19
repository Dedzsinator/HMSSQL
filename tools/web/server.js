const express = require('express');
const { spawn } = require('child_process');
const path = require('path');

const app = express();
const PORT = 3000;

// Start the HMSSQL daemon
const startDaemon = () => {
    const daemon = spawn('./daemon', [], {
        cwd: path.join(__dirname, '../../build/bin')
    });

    daemon.stdout.on('data', (data) => {
        console.log(`Daemon output: ${data}`);
    });

    daemon.stderr.on('data', (data) => {
        console.error(`Daemon error: ${data}`);
    });

    daemon.on('close', (code) => {
        console.log(`Daemon process exited with code ${code}`);
    });

    return daemon;
};

// Serve static files
app.use(express.static('static'));

// Proxy requests to the daemon
app.post('/query', express.text(), async (req, res) => {
    try {
        const response = await fetch('http://localhost:8080/query', {
            method: 'POST',
            body: req.body
        });
        const data = await response.json();
        res.json(data);
    } catch (error) {
        res.status(500).json({ 
            status: 'error', 
            message: error.message 
        });
    }
});

// Start the server
const server = app.listen(PORT, () => {
    console.log(`Web server running on http://localhost:${PORT}`);
    const daemon = startDaemon();
    
    // Clean up on server shutdown 
    process.on('SIGTERM', () => {
        daemon.kill();
        server.close();
    });
});