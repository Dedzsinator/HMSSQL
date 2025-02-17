document.addEventListener('DOMContentLoaded', () => {
    const queryInput = document.getElementById('queryInput');
    const executeBtn = document.getElementById('executeBtn');
    const createDbBtn = document.getElementById('createDbBtn');
    const queryResult = document.getElementById('queryResult');
    const databaseList = document.getElementById('databaseList');
    const tableList = document.getElementById('tableList');

    // Execute Query
    executeBtn.addEventListener('click', async () => {
        const query = queryInput.value;
        try {
            const response = await fetch('/query', {
                method: 'POST',
                body: query
            });
            const result = await response.json();
            displayResults(result);
        } catch (error) {
            console.error('Error:', error);
            queryResult.innerHTML = `<div class="error">Error: ${error.message}</div>`;
        }
    });

    // Create Database
    createDbBtn.addEventListener('click', async () => {
        const dbName = prompt('Enter database name:');
        if (dbName) {
            try {
                const response = await fetch('/query', {
                    method: 'POST',
                    body: `CREATE DATABASE ${dbName};`
                });
                const result = await response.json();
                if (result.status === 'success') {
                    loadDatabases();
                }
            } catch (error) {
                console.error('Error:', error);
            }
        }
    });

    // Display Query Results
    function displayResults(result) {
        if (result.status === 'error') {
            queryResult.innerHTML = `<div class="error">${result.message}</div>`;
            return;
        }

        if (result.result && result.result.length > 0) {
            const tables = result.result.map(table => {
                return `<div class="result-table-wrapper">${table}</div>`;
            }).join('');
            queryResult.innerHTML = tables;
        } else {
            queryResult.innerHTML = '<div>No results</div>';
        }
    }

    // Load Databases
    async function loadDatabases() {
        try {
            const response = await fetch('/query', {
                method: 'POST',
                body: '\\dt'
            });
            const result = await response.json();
            if (result.status === 'success') {
                const databases = result.result;
                databaseList.innerHTML = databases.map(db => 
                    `<li onclick="useDatabase('${db}')">${db}</li>`
                ).join('');
            }
        } catch (error) {
            console.error('Error:', error);
        }
    }

    // Initial load
    loadDatabases();
});

// Use Database
function useDatabase(dbName) {
    fetch('/query', {
        method: 'POST',
        body: `USE ${dbName};`
    })
    .then(response => response.json())
    .then(result => {
        if (result.status === 'success') {
            loadTables(dbName);
        }
    })
    .catch(error => console.error('Error:', error));
}

// Load Tables for Selected Database
async function loadTables(dbName) {
    try {
        const response = await fetch('/query', {
            method: 'POST',
            body: '\\dt'
        });
        const result = await response.json();
        if (result.status === 'success') {
            const tableList = document.getElementById('tableList');
            tableList.innerHTML = result.result.map(table => 
                `<li onclick="showTableStructure('${table}')">${table}</li>`
            ).join('');
        }
    } catch (error) {
        console.error('Error:', error);
    }
}