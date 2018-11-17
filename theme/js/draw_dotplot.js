
////////////////////////////////////////////////////////////
// Display correspondence matrix with D3.js

var dotplot = dotplot || {};

dotplot = {  
		
	drawDotplotPlotly: function(primMap, primLinkageGroup, secMap, secLinkageGroup, corres ) {
	
		var dp = {};
		dp['x'] = [];
		dp['y'] = [];
		dp['label'] = [];
		dp['annotation'] = [];
	    for (var ckey in corres) {
	    	//corres: {"X17_1500":[{"linkageGroup":"LGIII","position":"184.2", "feature_id": 1233554},{"linkageGroup":"LGIIIb.3","position":"74.5943", ..}],"
	    	if (corres.hasOwnProperty(ckey)) {
				markerCorres = corres[ckey];
				if (Object.keys(markerCorres).length >= 2 ) {
					// only use the marker if it pertains to either the primary or secondary linkage group (correspondences are for the whole map)
					if ((markerCorres[0].linkageGroup == primLinkageGroup) || (markerCorres[0].linkageGroup == secLinkageGroup)) {
					var xPos = parseInt(markerCorres[0].position);
					var yPos = parseInt(markerCorres[1].position);
					dp.x.push(xPos);
					dp.y.push(yPos);
					dp.label.push(ckey);
					dp.annotation.push({ x: xPos, y: yPos, text: "<a href = '"+Drupal.settings.baseUrl+markerCorres[0].feature_url+"'>{}</a>", 
						showarrow: false, xanchor:'center', yanchor:'center', opacity: 0, hovertext: ckey });
					}
				}
	    	}
	    };
	    
	    var markerSize = 10;
	    var trace1 = {
		  x: dp.x,
		  y: dp.y,
		  mode: 'markers',
		  type: 'scatter',
		  //text: dp.label, use annotation with hover over option instead of labels, so can specify a link.
		  marker: { 
			  size: markerSize,
              color: 'rgb(58, 118, 175)',
              line: { color: 'rgb(128, 158, 193)', /*121, 172, 207)',*/ width: 1}, 
			 },
		  //hoverinfo: "text", - default for hoverinfo, is to show position and text if available. If do not want position to appear, specifiy: none.
		  cliponaxis: false,
		};

		var data = [ trace1];

		var layout = {
		  
			width: 600, 
	        height: 600, 
			xaxis: {
				range: [0, (Math.max(...dp.x) + markerSize)],
				title: primMap+'<br>'+tripalMap.lGNameReduction(primLinkageGroup, primMap),
				layer: "below traces", 
			},

			yaxis: {
				range: [0, (Math.max(...dp.y) + markerSize)],
				title: secMap +'<br>'+tripalMap.lGNameReduction(secLinkageGroup, secMap),
				layer: "below traces",

			},
			hovermode:'closest',
			showlegend: false,
			annotations: dp.annotation, 
		};

    	
		Plotly.newPlot('select_fieldset_dotplot_svg', data, layout);	
		
	},		
		
	drawDotplot: function(primMap, primLinkageGroup, secMap, secLinkageGroup, corres ) {
		// return if there is no map
		if ((primMap === null ) || (secMap === null)) { 
			return;
		}

		var margin = {top: 75, right: 0, bottom: 0, left: 100};
	    var width = 600;
	    var height = 900;

		var svg = d3.select("#select_fieldset_dotplot_svg")
			.append("svg")
			.attr("class", "TripalMap")
	    	.attr("width", width)
	    	.attr("height", height);
			
		// correspondence matrix frame 
	    var cmFrame = svg.append("g")
        	.attr("id", "cmFrame")
        	.attr("transform", "translate("+ margin.left + "," + margin.top + ")")
        	.attr("visibility", "unhidden");
	    
	    // parse correspondences and put them into format: [{"x":67.91,"y":39.57},...]
	    var dp = [];
	    for (var ckey in corres) {
	    	//corres: {"X17_1500":[{"linkageGroup":"LGIII","position":"184.2"},{"linkageGroup":"LGIIIb.3","position":"74.5999999999999943"}],"
	    	if (corres.hasOwnProperty(ckey)) {
				markerCorres = corres[ckey];
				if (Object.keys(markerCorres).length >= 2 ) {
					var corresVal = {};
					var xPos = parseInt(markerCorres[0].position);
					corresVal["x"] = xPos;
					var yPos = parseInt(markerCorres[1].position);
					corresVal["y"] = yPos;
					dp.push(corresVal);
				}
	    	}
	    };
	    
        var options = {"dp": dp, "primMap": primMap, "primLinkageGroup": primLinkageGroup, 
        		"secMap": secMap, "secLinkageGroup": secLinkageGroup, "corres": corres};

        dotplot.draw(options, cmFrame);
	},
        
	draw: function(options, svg) {

		var data = options.dp;
    	var width = 250;
    	var height = 250;
    	var textColor = "#3b3b3b";
    	        	
    	if(!data) {
    		throw new Error('Please pass data');
    	}

    	// Create x label
    	var xValue = function(d) { return d.x;};
    	var xMin = d3.min(data, xValue);
    	var xMax = d3.max(data, xValue);
		var mapX = options.primMap; 
    	var lgX = options.primLinkageGroup;
    	lgX = tripalMap.lGNameReduction(lgX, mapX);
    	var xLabel = [mapX, lgX, "0 -"+xMax+" cM"];
    	
    	// x-axis
       	var xScale = d3.scaleLinear().range([0, width]);
    	// prevent dots overlapping the axis. Add buffer to data domain
    	var padding = 6;
    	xScale.domain([d3.min(data, xValue) - padding, d3.max(data, xValue) + padding]);

    	var xAxis = '';
    	if (tripalMap.d3VersionFour()) { 
    		xAxis = d3.axisTop(xScale);
    	}
    	else {
    		xAxis = d3.svg.axis().scale(xScale).orient("top");
    	}
    	xAxisSvg = svg.append("g")
        	.attr("class", "x axis");
    	xAxisSvg.call(xAxis);

    	var xAxisg = xAxisSvg.append("g")
       		.attr("fill", textColor)
    		.attr("transform", "translate(45,-60)");

    	xAxisg.selectAll("text")
			.data(xLabel)
    		.enter().append("text")
    		.attr("class", "label").attr("y", function(d,i) { return (i*18);})
 			.style("text-anchor", "start").style("font-size", "15px").style("line-height", "18px")
    		.text(function(d) {return d;});
    	
    	// Create y label
    	var yValue = function(d) { return d.y;};
    	var yMin = d3.min(data, yValue);
    	var yMax = d3.max(data, yValue);
    	var mapY = options.secMap;
    	var lgY = options.secLinkageGroup;  
    	lgY = tripalMap.lGNameReduction(lgY, mapY);
    	var yLabel = [mapY, lgY,"0 -"+yMax+" cM"];

    	// y-axis
    	var yScale = d3.scaleLinear().range([height, 0]);
    	// prevent dots overlapping axis. Add buffer to data domain
    	yScale.domain([d3.min(data, yValue)-padding, d3.max(data, yValue) + padding]);
    	
    	var yAxis = '';
    	if (tripalMap.d3VersionFour()) { 
       		yAxis = d3.axisLeft(yScale);
       	}
       	else {
       		yAxis = d3.svg.axis().scale(yScale).orient("left");
       	 }
    	
    	
    	yAxisSvg = svg.append("g")
    		.attr("class", "y axis");
    	yAxisSvg.call(yAxis);
    	       		
       	var yAxisg = yAxisSvg.append("g")
       		.attr("fill", textColor)
    		.attr("transform", "rotate(-90)");

       	var yAxisgy = -70;
    	yAxisg.selectAll("text")
			.data(yLabel)
    		.enter().append("text")
    		.attr("class", "label").attr("x", -180).attr("y", function(d, i) { return (yAxisgy + (i*18));})
 			.style("text-anchor", "start").style("font-size", "15px").style("line-height", "18px")
    		.text(function(d) {return d;});
    	
    	// remove ticks on y and x axis
  	  	svg.selectAll(".tick").remove();

  	  	// wrap around text that is too long
		svg.selectAll(".label")
      			.call(tripalMap.wrap, 280);
 
        	// draw dots
    	var xMap = function(d) { return xScale(xValue(d));};
    	var yMap = function(d) { return yScale(yValue(d));};
    	svg.selectAll(".dot")
    		.data(data)
    		.enter().append("circle")
    			.attr("class", "dot")
    			.attr("r", 5)
    			.attr("cx", xMap)
    			.attr("cy", yMap)
    			.style("fill", textColor); 
        	
	},   
	
};


