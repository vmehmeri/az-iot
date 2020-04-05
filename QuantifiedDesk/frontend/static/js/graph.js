Chart.defaults.global.defaultFontFamily = '-apple-system,system-ui,BlinkMacSystemFont,"Segoe UI",Roboto,"Helvetica Neue",Arial,sans-serif';

    Chart.defaults.global.defaultFontColor = '#292b2c';

    var ctx = document.getElementById("myBarChart");
    var myLineChart = new Chart(ctx, {
      type: 'bar',
      data: {
        labels: ["Standing", "Sitting"],
        datasets: [{
          label: "Time (minutes)",
          backgroundColor: "rgba(2,117,216,1)",
          borderColor: "rgba(2,117,216,1)",
          data: [0,0,0],
        }],
      },
      options: {
        scales: {
          xAxes: [{
            time: {
              unit: 'month'
            },
            gridLines: {
              display: false
            },
            ticks: {
              maxTicksLimit: 6
            }
          }],
        },
        legend: {
          display: false
        }
      }
    });

    // Configure Pusher instance
    const pusher = new Pusher('9bef95b942738c125933', {
        cluster: 'eu',
        encrypted: true
    });

    // Subscribe to poll trigger
    var graphChannel = pusher.subscribe('graph');

    // Listen to event
    graphChannel.bind('update', function(data) {
      myLineChart.data.datasets.forEach((dataset) => {
          dataset.data = data.units.split(","); 
      });
      myLineChart.update();
    });