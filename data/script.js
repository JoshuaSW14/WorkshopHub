// const ctx = document.getElementById("chart").getContext("2d");
// const myChart = new Chart(ctx, {
//   type: "line",
//   data: {
//     labels: [],
//     datasets: [
//       {
//         label: "Temperature",
//         backgroundColor: "rgba(161, 198, 247, 1)",
//         borderColor: "rgb(47, 128, 237)",
//         data: [],
//       },
//     ],
//   },
//   options: {
//     scales: {
//       yAxes: [
//         {
//           ticks: {
//             beginAtZero: true,
//           },
//         },
//       ],
//     },
//   },
// });

function convertRange(value, r1, r2) {
  return ((value - r1[0]) * (r2[1] - r2[0])) / (r1[1] - r1[0]) + r2[0];
}

// function updateChart(time, value) {
//   myChart.data.labels.push(time);
//   myChart.data.datasets.forEach((dataset) => {
//     dataset.data.push(value);
//   });
//   myChart.update();
// }

function fetchData() {
  var xmlHttp = new XMLHttpRequest();
  xmlHttp.onload = function () {
    if (xmlHttp.status != 200) {
      console.log(
        "GET ERROR: [" + xmlHttp.status + "] " + xmlHttp.responseText
      );
    } else {
      var espData = JSON.parse(xmlHttp.responseText);
      var x = espData.time;

      //updateChart(espData.time, espData.temperature);
      document.getElementById("temperature").style.width =
        convertRange(espData.temperature, [10, 30], [0, 100]) + "%";
      document.getElementById("temperature").innerHTML =
        espData.temperature + "°C";

      document.getElementById("humidity").style.width =
        convertRange(espData.humidity, [0, 100], [0, 100]) + "%";
      document.getElementById("humidity").innerHTML = espData.humidity;
      document.getElementById("co").style.width =
        convertRange(espData.co, [0, 5], [0, 100]) + "%";
      document.getElementById("co").innerHTML = espData.co;
      document.getElementById("alcohol").style.width =
        convertRange(espData.alcohol, [0, 5], [0, 100]) + "%";
      document.getElementById("alcohol").innerHTML = espData.alcohol;
      document.getElementById("co2").style.width =
        convertRange(espData.co2, [0, 1000], [0, 100]) + "%";
      document.getElementById("co2").innerHTML = espData.co2;
      document.getElementById("toluen").style.width =
        convertRange(espData.toluen, [0, 5], [0, 100]) + "%";
      document.getElementById("toluen").innerHTML = espData.toluen;
      document.getElementById("nh4").style.width =
        convertRange(espData.nh4, [0, 5], [0, 100]) + "%";
      document.getElementById("nh4").innerHTML = espData.nh4;
      document.getElementById("aceton").style.width =
        convertRange(espData.aceton, [0, 5], [0, 100]) + "%";
      document.getElementById("aceton").innerHTML = espData.aceton;

      window.setTimeout(fetchData, 5000);
    }
  };
  xmlHttp.onerror = function (e) {
    console.log("ERROR: [" + xmlHttp.status + "] " + xmlHttp.responseText);
    window.setTimeout(fetchData, 5000);
  };
  xmlHttp.open("GET", "/checkInput", true);
  xmlHttp.send();
}

// function fetchInitialData() {
//   var xmlHttp = new XMLHttpRequest();
//   xmlHttp.onload = function () {
//     if (xmlHttp.status != 200) {
//       console.log(
//         "GET ERROR: [" + xmlHttp.status + "] " + xmlHttp.responseText
//       );
//     } else {
//       var espData = JSON.parse(xmlHttp.responseText);
//       document.getElementById("sensorID").innerHTML = espData.sensorid;
//       window.setTimeout(fetchData, 5000);
//     }
//   };
//   xmlHttp.onerror = function (e) {
//     console.log("ERROR: [" + xmlHttp.status + "] " + xmlHttp.responseText);
//     window.setTimeout(fetchData, 5000);
//   };
//   xmlHttp.open("GET", "/sensor", true);
//   xmlHttp.send();
// }

//fetchInitialData();
fetchData();
