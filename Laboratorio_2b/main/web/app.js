const redInput = document.getElementById("red");
const greenInput = document.getElementById("green");
const blueInput = document.getElementById("blue");

const redValue = document.getElementById("red-value");
const greenValue = document.getElementById("green-value");
const blueValue = document.getElementById("blue-value");

const ledStatus = document.getElementById("led-status");
const colorPreview = document.getElementById("color-preview");
const sendButton = document.getElementById("send-button");

function getColor() {
  return {
    r: Number(redInput.value),
    g: Number(greenInput.value),
    b: Number(blueInput.value)
  };
}

function updatePreview() {
  const { r, g, b } = getColor();

  redValue.textContent = r;
  greenValue.textContent = g;
  blueValue.textContent = b;

  colorPreview.style.backgroundColor = `rgb(${r}, ${g}, ${b})`;
  ledStatus.textContent = `Color actual: R=${r}, G=${g}, B=${b}`;
}

async function sendColor() {
  const color = getColor();

  try {
    await fetch("/led", {
      method: "POST",
      headers: {
        "Content-Type": "application/json"
      },
      body: JSON.stringify(color)
    });

    updatePreview();
  } catch (error) {
    ledStatus.textContent = "Error FATAL al enviar el color al ESP32";
    console.error(error);
  }
}

redInput.addEventListener("input", updatePreview);
greenInput.addEventListener("input", updatePreview);
blueInput.addEventListener("input", updatePreview);
sendButton.addEventListener("click", sendColor);

updatePreview();
