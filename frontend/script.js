// Parse the plain-text travel plan into structured data
function parseTravelPlan(output) {
    const lines = output.split('\n').filter(l => l.trim());
    const segments = [];
    const totals = { distance: '', time: '', cost: '' };
    
    const headerIdx = lines.findIndex(l => l.includes('From') && l.includes('To'));
    if (headerIdx >= 0) {
        for (let i = headerIdx + 2; i < lines.length; i++) {
            const line = lines[i];
            if (line.trim().startsWith('-')) break;
            
            const from = line.substr(0, 25).trim();
            const to = line.substr(25, 25).trim();
            const mode = line.substr(50, 10).trim();
            const distance = line.substr(60, 10).trim();
            const time = line.substr(70, 10).trim();
            const cost = line.substr(80).trim();
            
            segments.push({ from, to, mode, distance, time, cost });
        }
    }
    
    const totalLine = lines.find(l => l.includes('Total:'));
    if (totalLine) {
        const parts = totalLine.split('|').map(p => p.trim());
        if (parts.length === 3) {
            totals.distance = parts[0].replace(/^.*Total:\s*/, '');
            totals.time = parts[1];
            totals.cost = parts[2];
        }
    }
    
    return { segments, totals };
}

// Display results in the UI
function displayResults(data) {
    const { segments, totals } = data;
    const resultsElement = document.getElementById('results');
    const emptyStateElement = document.getElementById('empty-state');
    const resultsBodyElement = document.getElementById('results-body');
    const resultsSummaryElement = document.getElementById('results-summary');
    
    if (!segments.length) {
        resultsElement.classList.add('hidden');
        document.getElementById('error-message').classList.remove('hidden');
        document.getElementById('error-message').textContent = 'No route found between the specified cities.';
        emptyStateElement.classList.add('hidden');
        return;
    }
    
    // Clear previous results
    resultsBodyElement.innerHTML = '';
    resultsSummaryElement.innerHTML = '';
    
    // Add segments to table
    segments.forEach(segment => {
        // Get icon based on mode
        let modeIcon = '';
        switch(segment.mode.toLowerCase()) {
            case 'flight':
                modeIcon = '<i class="fas fa-plane"></i>';
                break;
            case 'train':
                modeIcon = '<i class="fas fa-train"></i>';
                break;
            case 'bus':
                modeIcon = '<i class="fas fa-bus"></i>';
                break;
            case 'car':
                modeIcon = '<i class="fas fa-car"></i>';
                break;
            default:
                modeIcon = '<i class="fas fa-route"></i>';
        }
        
        const row = document.createElement('tr');
        row.innerHTML = `
            <td>${segment.from}</td>
            <td>${segment.to}</td>
            <td>${modeIcon} ${segment.mode}</td>
            <td>${segment.distance}</td>
            <td>${segment.time.substr(2)}</td>
            <td>${segment.cost.substr(3)}</td>
        `;
        resultsBodyElement.appendChild(row);
    });
    
    // Add totals summary
    resultsSummaryElement.innerHTML = `
        <div class="summary-item">
            <i class="fas fa-road summary-icon"></i>
            <span>Total Distance: ${totals.distance}</span>
        </div>
        <div class="summary-item">
            <i class="fas fa-clock summary-icon"></i>
            <span>Total Time: ${totals.time}</span>
        </div>
        <div class="summary-item">
            <i class="fas fa-money-bill-wave summary-icon"></i>
            <span>Total Cost: ${totals.cost}</span>
        </div>
    `;
    
    // Show results
    resultsElement.classList.remove('hidden');
    emptyStateElement.classList.add('hidden');
    document.getElementById('error-message').classList.add('hidden');
}

// Load city list for autocomplete
function loadCitiesDatalist() {
    fetch('cities.csv')
        .then(resp => resp.text())
        .then(text => {
            const lines = text.split('\n').filter(l => l.trim());
            const datalist = document.getElementById('cities');
            lines.forEach(line => {
                const parts = line.split(',');
                if (parts.length >= 2) {
                    const country = parts[0].trim();
                    const city = parts[1].trim();
                    const option = document.createElement('option');
                    option.value = city + ', ' + country;
                    datalist.appendChild(option);
                }
            });
        })
        .catch(err => console.error('Error loading cities for autocomplete:', err));
}

// Handle number of stops change
function updateStopsContainer() {
    const numStops = parseInt(document.getElementById('num-stops').value) || 0;
    const stopsContainer = document.getElementById('stops-container');
    
    // Clear existing stops
    stopsContainer.innerHTML = '';
    
    // Add stop entries
    for (let i = 0; i < numStops; i++) {
        addStopEntry(i + 1);
    }
    
    // Update add-stop button visibility
    document.getElementById('add-stop-btn').style.display = numStops > 0 ? 'flex' : 'none';
}

// Add a new stop entry
function addStopEntry(stopNumber) {
    const stopsContainer = document.getElementById('stops-container');
    const stopEntry = document.createElement('div');
    stopEntry.className = 'stop-entry';
    stopEntry.innerHTML = `
        <div class="stop-header">
            <div class="stop-number">${stopNumber}</div>
            <div class="stop-title">Stop ${stopNumber}</div>
        </div>
        <button type="button" class="remove-stop" onclick="removeStopEntry(this)" aria-label="Remove stop">
            <i class="fas fa-times"></i>
        </button>
        
        <div class="stop-row">
            <div>
                <label for="stop-city-${stopNumber}">City or Country</label>
                <input type="text" id="stop-city-${stopNumber}" placeholder="e.g., Seoul, South Korea" list="cities" required>
            </div>
            <div>
                <label for="stop-mode-${stopNumber}">Transport Mode</label>
                <select id="stop-mode-${stopNumber}" required>
                    <option value="Flight">Flight</option>
                    <option value="Train">Train</option>
                    <option value="Bus">Bus</option>
                    <option value="Car">Car</option>
                </select>
            </div>
        </div>
    `;
    stopsContainer.appendChild(stopEntry);
    
    // Update the number of stops input
    document.getElementById('num-stops').value = document.querySelectorAll('.stop-entry').length;
}

// Remove a stop entry
function removeStopEntry(button) {
    const stopEntry = button.parentNode;
    stopEntry.parentNode.removeChild(stopEntry);
    
    // Update stop numbers
    const stopEntries = document.querySelectorAll('.stop-entry');
    stopEntries.forEach((entry, index) => {
        const stopNumber = index + 1;
        entry.querySelector('.stop-number').textContent = stopNumber;
        entry.querySelector('.stop-title').textContent = `Stop ${stopNumber}`;
        
        // Update IDs
        const cityInput = entry.querySelector('input[id^="stop-city-"]');
        const modeSelect = entry.querySelector('select[id^="stop-mode-"]');
        
        cityInput.id = `stop-city-${stopNumber}`;
        modeSelect.id = `stop-mode-${stopNumber}`;
    });
    
    // Update the number of stops input
    document.getElementById('num-stops').value = stopEntries.length;
}

// Plan trip
async function planTrip(event) {
    if (event) event.preventDefault();
    
    const startCity = document.getElementById('start-city').value.trim();
    const endCity = document.getElementById('end-city').value.trim();
    
    if (!startCity || !endCity) {
        document.getElementById('error-message').textContent = 'Please enter both starting and destination cities.';
        document.getElementById('error-message').classList.remove('hidden');
        return;
    }
    
    // Hide results and show loading spinner
    document.getElementById('results').classList.add('hidden');
    document.getElementById('empty-state').classList.add('hidden');
    document.getElementById('error-message').classList.add('hidden');
    document.getElementById('loading').classList.remove('hidden');
    
    // Get stops data
    const stopEntries = document.querySelectorAll('.stop-entry');
    const stops = Array.from(stopEntries).map((entry, index) => {
        const city = entry.querySelector(`input[id^="stop-city-"]`).value.trim();
        const mode = entry.querySelector(`select[id^="stop-mode-"]`).value;
        return { city, mode };
    });
    
    const numStops = stops.length;
    
    try {
        const response = await fetch('http://localhost:3000/plan-trip', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ startCity, numStops, stops, endCity })
        });
        
        const data = await response.json();
        
        // Hide loading spinner
        document.getElementById('loading').classList.add('hidden');
        
        if (data.error) {
            document.getElementById('error-message').textContent = `Error: ${data.error}`;
            document.getElementById('error-message').classList.remove('hidden');
        } else {
            // Parse and display results
            const parsedData = parseTravelPlan(data.output);
            displayResults(parsedData);
        }
    } catch (error) {
        document.getElementById('loading').classList.add('hidden');
        document.getElementById('error-message').textContent = `Error: Failed to connect to server. ${error.message}`;
        document.getElementById('error-message').classList.remove('hidden');
    }
}

// Initialize
document.addEventListener('DOMContentLoaded', function() {
    // Load cities datalist
    loadCitiesDatalist();
    
    // Initialize stops container
    updateStopsContainer();
    
    // Event listeners
    document.getElementById('num-stops').addEventListener('change', updateStopsContainer);
    document.getElementById('add-stop-btn').addEventListener('click', function() {
        const numStops = document.querySelectorAll('.stop-entry').length;
        addStopEntry(numStops + 1);
    });
    
    document.getElementById('trip-form').addEventListener('submit', planTrip);
    
    // Initialize add-stop button visibility
    document.getElementById('add-stop-btn').style.display = 'none';
});

// Make removeStopEntry available globally
window.removeStopEntry = removeStopEntry;