$(document).ready(function(){
    var dataTable = $("#dataTable").DataTable()
    var activityChannel = pusher.subscribe('activity');
    activityChannel.bind('add', function(data) {
    var date = new Date();
    dataTable.row.add([
        data.date,
        data.event,
        data.time
      ]).draw( false );
    });
  });