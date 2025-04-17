const express = require('express');
const { execFile } = require('child_process');
const cors = require('cors');
const path = require('path');

const app = express();
const port = 3000;

app.use(cors());
app.use(express.json());

// Serve static files from the frontend directory
app.use(express.static(path.join(__dirname, 'frontend')));

// Also serve the root directory for CSV files
app.use(express.static(__dirname));

// Add a specific route for the homepage
app.get('/', (req, res) => {
  res.sendFile(path.join(__dirname, 'frontend', 'index.html'));
});

// Endpoint to handle travel planning
app.post('/plan-trip', (req, res) => {
    const { startCity, numStops, stops, endCity } = req.body;

    // Prepare input for the C++ program
    let input = `${startCity}\n${numStops}\n`;
    for (let i = 0; i < numStops; i++) {
        input += `${stops[i].city}\n${stops[i].mode}\n`;
    }
    input += `${endCity}\n`;
    if (numStops > 0) {
        input += `${stops[numStops - 1].mode}\n`; // Mode for final leg
    }

    // Run the C++ executable
    const execPath = path.join(__dirname, 'travelPlanner.exe');
    const child = execFile(execPath, [], { cwd: __dirname }, (error, stdout, stderr) => {
        if (error) {
            console.error('Error executing C++ program:', stderr);
            return res.status(500).json({ error: 'Failed to plan trip' });
        }

        // Parse the output
        res.json({ output: stdout });
    });

    // Send input to the C++ program's stdin
    child.stdin.write(input);
    child.stdin.end();
});

app.listen(port, () => {
    console.log(`Server running at http://localhost:${port}`);
});


