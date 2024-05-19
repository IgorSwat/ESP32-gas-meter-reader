function fetchAndDisplayImage() {
    fetch('http://192.168.43.209/data', {
        method: 'GET'
    })
        .then(response => {
            if (response.ok) {
                return response.arrayBuffer(); // Extract the response body as an ArrayBuffer
            } else {
                throw new Error('Network response was not ok.');
            }
        })
        .then(arrayBuffer => {
            // Convert the ArrayBuffer to a base64-encoded string
            const byteArray = new Uint8Array(arrayBuffer);
            let binaryString = '';
            for (let i = 0; i < byteArray.byteLength; i++) {
                binaryString += String.fromCharCode(byteArray[i]);
            }
            const base64Data = btoa(binaryString);

            // Create a data URL with the base64-encoded image data
            const imageUrl = `data:image/jpeg;base64,${base64Data}`;

            // Display the image on the webpage
            const imageContainer = document.getElementById('imageContainer');
            imageContainer.innerHTML = ''; // Clear previous image (if any)

            const imgElement = document.createElement('img');
            imgElement.src = imageUrl;
            imgElement.style.maxWidth = '75%'; // Ensure the image fits within container
            imageContainer.appendChild(imgElement);
        })
        .catch(error => {
            console.error('Error fetching and displaying image:', error);
            alert('Failed to fetch image. Please try again.');
        });
}

