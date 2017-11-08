<html>
<head>
<script src="https://code.jquery.com/jquery-3.1.1.min.js"></script>
<script src="https://code.highcharts.com/stock/highstock.js"></script>
<script src="https://code.highcharts.com/stock/modules/exporting.js"></script>

<?php
	header("Refresh: 60;");
?>

<?php 
	error_reporting(0);
	$conn=new mysqli("localhost","username","password","database_name");
	if(mysqli_connect_errno())
	{
	echo "Failed to connect to SQL".mysqli_connect_errno();
	}
	else
	{
		//echo "Connection established";
	}
	
//Query to fetch the id of the recent data		
$sql = "SELECT id FROM tabel_name ORDER BY id DESC LIMIT 1;";
$result=mysqli_query($conn,$sql);
while($row = mysqli_fetch_array($result)) 
{
$val = $row['id'];
	
}

//to go a week back from current
$point=$val-103680;

$sql="SELECT time FROM tabel_name WHERE id='".$point."'";
$result=mysqli_query($conn,$sql);
while($row = mysqli_fetch_array($result)) 
{
$time = $row['time'];
		
}
$timestamp_UNIX = strtotime($time);

$tim=date("Y-m-d\TH:i:s\Z",$timestamp_UNIX);

$y = substr($time, 0, 4);
$m=substr($time, 5, 2);
$d=substr($time, 8, 2);
$h=substr($time, 11, 2);
$min=substr($time, 14, 2);

$yy=(int)$y;
$mm=(int)$m;
$dd=(int)$d;
$hh=(int)$h;
$minin=(int)$min;

	// query to get all the data for a week
	$sql="Select time,Humidity from tabel_name Where id > '".$point."'";
	
	$result=mysqli_query($conn,$sql);
	
	
	
	$val=array();
	$tim=array();
	
	

//store data in the variable fetched from database	
while($row = mysqli_fetch_array($result)) 
{
	
	$value = $row['Humidity'];
    $timestamp = strtotime($row['time']) * 1000;
    
	$val[]=$value;
	$tim[]=$timestamp;
	
	
}
	
	
?>


</head>

<body>
<div id="container" style="height: 400px; min-width: 310px"></div>
</body>

<script>


	//convert php variables to javascript variable
	var y = <?php echo $yy ?>;
	var m = <?php echo $mm ?>;
	m=m-1;
	var d=<?php echo $dd ?>;
	
	var h=<?php echo $hh ?>;
	var min=<?php echo $minin ?>;


// Create the chart
 
Highcharts.setOptions({
    global: {
        useUTC: true
    }
});

	
	
Highcharts.stockChart('container', 
{

        chart: {
            height: 500
        },
      
		xAxis: {
			title:{text: 'Date Time'},

			categories:[<?php echo join($tim,',')?>],
			type: 'datetime',
		},
	rangeSelector: {
            allButtonsEnabled: true,
            buttons: [{
                type: 'day',
                count:1,
                text: '1Day'
            }, {
                type: 'day',
                count: 4,
                text: '4days'
            }, {
                type: 'week',
                count: 1,
				text: 'Week'
            }, {
                type: 'all',
                text: 'all'
            }],
            buttonTheme: {
                width: 60
            },
            selected: 0
        },
		yAxis: {
			title:{text: 'Reletive Humidity'}
		},
		
        title: {
            text: 'Packaging Lab: Node 1'
        },

        subtitle: {
            text: 'Humidity sensor'
        },

        _navigator: {
            enabled: false
        },

		
		tooltip: {
        valueDecimals: 1,
        valueSuffix: ' %'
		},
    series: [{
		data:[<?php echo join($val,',')?>],	
	name: 'Humidity',
       

        //point at which the x-axis should start
        pointStart: Date.UTC(y, m, d, h, min),
        pointInterval: 5 * 1000 // has to be approx 5 sec-
        
	}]
    });

</script>
</html>
