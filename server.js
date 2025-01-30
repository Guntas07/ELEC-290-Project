const express = require('express');
const { SerialPort } = require('serialport');
const { ReadlineParser } = require('@serialport/parser-readline');
const path = require('path'); // For resolving file paths

// Create an Express app
const app = express();

// Serial port configuration
const port = new SerialPort({
  path: 'COM3', // Replace with the correct COM port for your device
  baudRate: 9600
});

// Create a parser for reading line-by-line data from the serial port
const parser = port.pipe(new ReadlineParser({ delimiter: '\n' }));

// Global variables to store motion and card scan status
let motionDetected = false;
let cardScannedInLast10Seconds = false;
let lockStatus = '🔒'; // Default to locked

// Activity log array
const activityLog = [];
let lastScanTime = 0; // Timestamp of the last fob scan
let lastMotionLogTime = 0; // Timestamp of the last motion log

// Function to check if data is valid JSON
function isValidJSON(str) {
  try {
    JSON.parse(str);
    return true;
  } catch (e) {
    return false;
  }
}

// Handle incoming data from the serial port
parser.on('data', (data) => {
  if (isValidJSON(data)) {
    const parsedData = JSON.parse(data);
    console.log("Parsed JSON data:", parsedData);

    // Update the status variables based on the incoming JSON
    motionDetected = parsedData.motionDetected;
    cardScannedInLast10Seconds = parsedData.cardScannedInLast10Seconds;

    const currentTime = Date.now();

    // Update lock status and log activity if a new fob scan is detected
    if (cardScannedInLast10Seconds && currentTime - lastScanTime > 10000) { // 10-second debounce
      lockStatus = '🔓';
      lastScanTime = currentTime;

      activityLog.push({
        time: new Date().toLocaleString(),
        user: 'Viktor', // Hardcoded user for now
        method: 'Fob Scan'
      });
    } else if (!cardScannedInLast10Seconds) {
      lockStatus = '🔒';
    }

    // Log "Unknown Person" for motion detection if it's been more than 20 seconds since the last log
    if (motionDetected && currentTime - lastMotionLogTime > 20000) { // 20-second debounce
      lastMotionLogTime = currentTime;

      activityLog.push({
        time: new Date().toLocaleString(),
        user: 'Unknown Person',
        method: 'Denied Entry'
      });
    }
  } else {
    console.log("Received non-JSON data:", data);
  }
});

// Endpoint to get the current motion and card scan status
app.get('/status', (req, res) => {
  res.json({
    motionDetected,
    cardScannedInLast10Seconds,
    lockStatus
  });
});

// Endpoint to get the activity log
app.get('/log', (req, res) => {
  res.json(activityLog);
});

// Serve static files from the 'public' directory
app.use(express.static('public'));

// If the public directory doesn't exist, serve a fallback HTML file
app.get('/', (req, res) => {
  res.sendFile(path.join(__dirname, 'public', 'index.html'));
});

// Start the Express server
const serverPort = 3000; // Change this port if needed
app.listen(serverPort, () => {
  console.log(`Server running on http://localhost:${serverPort}`);
});

// Handle serial port errors
port.on('error', (err) => {
  console.error('Error with serial port:', err);
});
