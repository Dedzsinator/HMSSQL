const initGraph = () => {
    const cy = cytoscape({
        container: document.getElementById('graphContainer'),
        style: [
            {
                selector: 'node',
                style: {
                    'background-color': '#3498db',
                    'label': 'data(label)'
                }
            },
            {
                selector: 'edge',
                style: {
                    'width': 3,
                    'line-color': '#34495e',
                    'target-arrow-color': '#34495e',
                    'target-arrow-shape': 'triangle'
                }
            }
        ]
    });

    return cy;
};

// Initialize table designer grid
const initColumnGrid = new gridjs.Grid({
    columns: ['Name', 'Type', 'Nullable', 'Key', 'Default', 'Actions'],
    data: [],
    className: {
        table: 'w-full'
    }
}).render(document.getElementById('columnGrid'));

// Tab switching
document.querySelectorAll('.tab-btn').forEach(btn => {
    btn.addEventListener('click', () => {
        document.querySelectorAll('.tab-btn').forEach(b => b.classList.remove('active'));
        document.querySelectorAll('.view-content').forEach(v => v.classList.add('hidden'));
        btn.classList.add('active');
        document.getElementById(`${btn.dataset.view}View`).classList.remove('hidden');
    });
});

// Live data updates
const setupLiveUpdates = (tableName) => {
    setInterval(async () => {
        const response = await fetch('/query', {
            method: 'POST',
            body: `SELECT * FROM ${tableName};`
        });
        const data = await response.json();
        updateLiveDataGrid(data.results);
    }, 2000);
};

// Update graph with table relations
const updateGraphData = async () => {
    const response = await fetch('/query', {
        method: 'POST',
        body: '\\dc'
    });
    const data = await response.json();
    
    const elements = [];
    data.databases.forEach(db => {
        elements.push({
            data: { id: db.name, label: db.name }
        });
        db.tables.forEach(table => {
            elements.push({
                data: {
                    source: db.name,
                    target: `${db.name}.${table}`,
                    label: 'contains'
                }
            });
        });
    });

    cy.elements().remove();
    cy.add(elements);
    cy.layout({ name: 'cose' }).run();
};

document.addEventListener('DOMContentLoaded', () => {
    const cy = initGraph();
    updateGraphData();

    // Get DOM elements
    const createDbBtn = document.getElementById('createDbBtn');
    const executeBtn = document.getElementById('executeBtn');
    const queryInput = document.getElementById('queryInput');
    const queryResult = document.getElementById('queryResult');
    const databaseList = document.getElementById('databaseList');
    const tableList = document.getElementById('tableList');

    // Create Database button handler
    createDbBtn.addEventListener('click', async () => {
        const dbName = prompt('Enter database name:');
        if (dbName) {
            try {
                const response = await fetch('/query', {
                    method: 'POST',
                    body: `CREATE DATABASE ${dbName};`
                });
                const data = await response.json();
                if (data.status === 'success') {
                    updateDatabaseList();
                }
            } catch (error) {
                console.error('Error:', error);
            }
        }
    });

    // Execute query button handler
    executeBtn.addEventListener('click', async () => {
        const query = queryInput.value.trim();
        if (query) {
            try {
                const response = await fetch('/query', {
                    method: 'POST',
                    body: query
                });
                const data = await response.json();
                displayResults(data);
            } catch (error) {
                console.error('Error:', error);
                queryResult.innerHTML = `<div class="text-red-500">Error: ${error.message}</div>`;
            }
        }
    });

    // Update database list
    async function updateDatabaseList() {
        try {
            const response = await fetch('/query', {
                method: 'POST',
                body: '\\dc'
            });
            const data = await response.json();
            renderDatabaseList(data.databases || []);
        } catch (error) {
            console.error('Error:', error);
        }
    }

    // Render results in the result div
    function displayResults(data) {
        if (data.error) {
            queryResult.innerHTML = `<div class="text-red-500">${data.error}</div>`;
            return;
        }

        if (data.results && data.results.length > 0) {
            const table = document.createElement('table');
            table.className = 'w-full border-collapse';
            
            // Create header
            const header = document.createElement('tr');
            Object.keys(data.results[0]).forEach(key => {
                const th = document.createElement('th');
                th.className = 'border border-accent p-2 text-left';
                th.textContent = key;
                header.appendChild(th);
            });
            table.appendChild(header);

            // Create rows
            data.results.forEach(row => {
                const tr = document.createElement('tr');
                Object.values(row).forEach(value => {
                    const td = document.createElement('td');
                    td.className = 'border border-accent p-2';
                    td.textContent = value;
                    tr.appendChild(td);
                });
                table.appendChild(tr);
            });

            queryResult.innerHTML = '';
            queryResult.appendChild(table);
        } else {
            queryResult.innerHTML = '<div class="italic">No results</div>';
        }
    }

    // Initial load
    updateDatabaseList();
});