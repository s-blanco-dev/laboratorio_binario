<<<<<<< HEAD
const redSlider = document.getElementById("red");
const greenSlider = document.getElementById("green");
const blueSlider = document.getElementById("blue");
=======
const redInput = document.getElementById("red");
const greenInput = document.getElementById("green");
const blueInput = document.getElementById("blue");
>>>>>>> 3fd9b1f51ddfbc881e709092e33b166b430363e9

const redValue = document.getElementById("red-value");
const greenValue = document.getElementById("green-value");
const blueValue = document.getElementById("blue-value");

const ledStatus = document.getElementById("led-status");
const colorPreview = document.getElementById("color-preview");
const sendButton = document.getElementById("send-button");

<<<<<<< HEAD
function updateView(r, g, b) {
  redSlider.value = r;
  greenSlider.value = g;
  blueSlider.value = b;
=======
function getColor() {
  return {
    r: Number(redInput.value),
    g: Number(greenInput.value),
    b: Number(blueInput.value)
  };
}

function updatePreview() {
  const { r, g, b } = getColor();
>>>>>>> 3fd9b1f51ddfbc881e709092e33b166b430363e9

  redValue.textContent = r;
  greenValue.textContent = g;
  blueValue.textContent = b;

<<<<<<< HEAD
  ledStatus.textContent = `Color actual: RGB(${r}, ${g}, ${b})`;
  colorPreview.style.backgroundColor = `rgb(${r}, ${g}, ${b})`;
}

async function loadLedState() {
  try {
    const response = await fetch("/led");

    if (!response.ok) {
      throw new Error("No se pudo obtener el estado del LED");
    }

    const data = await response.json();

    updateView(data.r, data.g, data.b);
  } catch (error) {
    console.error("Error al consultar GET /led:", error);
    ledStatus.textContent = "Color actual: error al consultar el ESP32";
  }
}

async function sendColor() {
  const r = parseInt(redSlider.value, 10);
  const g = parseInt(greenSlider.value, 10);
  const b = parseInt(blueSlider.value, 10);
=======
  colorPreview.style.backgroundColor = `rgb(${r}, ${g}, ${b})`;
  ledStatus.textContent = `Color actual: R=${r}, G=${g}, B=${b}`;
}

async function sendColor() {
  const color = {
    r: Number(document.getElementById("red").value),
    g: Number(document.getElementById("green").value),
    b: Number(document.getElementById("blue").value)
  };
>>>>>>> 3fd9b1f51ddfbc881e709092e33b166b430363e9

  try {
    const response = await fetch("/led", {
      method: "POST",
      headers: {
<<<<<<< HEAD
        "Content-Type": "application/json",
      },
      body: JSON.stringify({ r, g, b }),
    });

    if (!response.ok) {
      throw new Error("No se pudo actualizar el LED");
    }

    updateView(r, g, b);
  } catch (error) {
    console.error("Error al enviar POST /led:", error);
    ledStatus.textContent = "Color actual: error al enviar color";
  }
}

function updateSliderLabelsOnly() {
  const r = parseInt(redSlider.value, 10);
  const g = parseInt(greenSlider.value, 10);
  const b = parseInt(blueSlider.value, 10);

  redValue.textContent = r;
  greenValue.textContent = g;
  blueValue.textContent = b;
  colorPreview.style.backgroundColor = `rgb(${r}, ${g}, ${b})`;
}

redSlider.addEventListener("input", updateSliderLabelsOnly);
greenSlider.addEventListener("input", updateSliderLabelsOnly);
blueSlider.addEventListener("input", updateSliderLabelsOnly);

sendButton.addEventListener("click", sendColor);

window.addEventListener("load", loadLedState);
=======
        "Content-Type": "application/json"
      },
      body: JSON.stringify(color)
    });

    if (!response.ok) {
      throw new Error("Error al enviar color");
    }

    console.log("Color enviado:", color);
  } catch (error) {
    console.error("Error:", error);
  }
}

redInput.addEventListener("input", updatePreview);
greenInput.addEventListener("input", updatePreview);
blueInput.addEventListener("input", updatePreview);
sendButton.addEventListener("click", sendColor);

updatePreview();
>>>>>>> 3fd9b1f51ddfbc881e709092e33b166b430363e9
