const ws = new WebSocket('ws://' + window.location.hostname + '/ws');
const video = document.getElementById('video');

ws.onopen = function() {
    console.log("Connected to WebSocket");
};

ws.onmessage = function(event) {
    // Handle incoming messages if needed
};

document.addEventListener('keydown', function(event) {
    switch (event.key) {
        case 'w': ws.send('W'); break; // Move forward
        case 's': ws.send('S'); break; // Move backward
        case 'a': ws.send('A'); break; // Turn left
        case 'd': ws.send('D'); break; // Turn right
    }
});

// Function to toggle light (to be implemented)
function toggleLight() {
    ws.send('toggle_light');
}

// Function to capture photo
function capturePhoto() {
    ws.send('capture');
}
