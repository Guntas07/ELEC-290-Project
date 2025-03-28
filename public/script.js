// Handle navigation between pages
function showPage(pageId) {
    const pages = document.querySelectorAll('.page');
    pages.forEach(page => page.classList.add('hidden'));
    document.getElementById(pageId).classList.remove('hidden');
}

// Fetch and update the lock status
function updateLockStatus() {
    const lockStatusElement = document.getElementById('lock-status');
    fetch('http://localhost:3000/status')
        .then(response => {
            if (!response.ok) {
                throw new Error(`HTTP error! Status: ${response.status}`);
            }
            return response.json();
        })
        .then(data => {
            // Update lock status dynamically
            lockStatusElement.textContent = data.cardScannedInLast10Seconds ? '🔓' : '🔒';
        })
        .catch(error => {
            console.error('Error fetching lock status:', error);
            lockStatusElement.textContent = 'Error Fetching Lock Status';
        });
}

// Fetch and update the activity log
function updateActivityLog() {
    const tableBody = document.getElementById('log-entries');
    fetch('http://localhost:3000/log')
        .then(response => {
            if (!response.ok) {
                throw new Error(`HTTP error! Status: ${response.status}`);
            }
            return response.json();
        })
        .then(logEntries => {
            // Clear the table body
            tableBody.innerHTML = '';
            // Populate the log entries dynamically
            logEntries.forEach(entry => {
                const row = `<tr>
                    <td>${entry.time}</td>
                    <td>${entry.user}</td>
                    <td>${entry.method}</td>
                </tr>`;
                tableBody.insertAdjacentHTML('beforeend', row);
            });
        })
        .catch(error => {
            console.error('Error fetching activity log:', error);
        });
}

// Send a request to unlock the door
function unlockDoor() {
    fetch('http://localhost:3000/status', { method: 'POST' })
        .then(response => {
            if (!response.ok) {
                throw new Error(`HTTP error! Status: ${response.status}`);
            }
            return response.json();
        })
        .then(data => {
            alert(data.status);
        })
        .catch(error => {
            console.error('Error unlocking door:', error);
            alert('Failed to unlock the door.');
        });
}

// Show emergency modal
const emergencyBtn = document.getElementById('emergency-btn');
const emergencyModal = document.getElementById('emergency-modal');
const closeEmergencyBtn = document.getElementById('close-emergency-btn');
const confirmEmergencyBtn = document.getElementById('confirm-emergency-btn');
const cancelEmergencyBtn = document.getElementById('cancel-emergency-btn');

emergencyBtn.addEventListener('click', () => {
    emergencyModal.style.display = 'block';
});

closeEmergencyBtn.addEventListener('click', () => {
    emergencyModal.style.display = 'none';
});

cancelEmergencyBtn.addEventListener('click', () => {
    emergencyModal.style.display = 'none';
});

confirmEmergencyBtn.addEventListener('click', () => {
    alert('Authorities have been notified!');
    emergencyModal.style.display = 'none';
});

// Fetch motion and card scan status
function fetchStatus() {
    const statusElement = document.getElementById('status');

    fetch('http://localhost:3000/status')
        .then(response => {
            if (!response.ok) {
                throw new Error(`HTTP error! Status: ${response.status}`);
            }
            return response.json();
        })
        .then(data => {
            // Display motion and card scan status
            const motionStatus = data.motionDetected ? 'Motion Detected' : 'No Motion';
            const cardStatus = data.cardScannedInLast10Seconds ? 'Card Scanned' : 'No Card Scanned in Last 10s';

            // Update the DOM with the fetched data
            statusElement.innerHTML = `
                <p><strong>Motion Status:</strong> ${motionStatus}</p>
                <p><strong>Card Scan Status:</strong> ${cardStatus}</p>
            `;
        })
        .catch(error => {
            console.error('Error fetching status:', error);
            statusElement.innerHTML = 'Error fetching status';
        });
}

// Periodically update lock status and activity log every 1 second
setInterval(() => {
    updateLockStatus();
    updateActivityLog();
}, 1000);

// Initial setup on page load
document.addEventListener('DOMContentLoaded', () => {
    updateLockStatus();
    updateActivityLog();

    // Unlock button functionality
    const unlockBtn = document.getElementById('unlock-btn');
    unlockBtn.addEventListener('click', unlockDoor);

    // Initial fetch for motion and card scan status
    fetchStatus();
});
