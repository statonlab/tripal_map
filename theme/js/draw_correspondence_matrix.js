
////////////////////////////////////////////////////////////
// Display correspondence matrix with D3.js

var correspondenceMatrix = correspondenceMatrix || {};

correspondenceMatrix = {  
		
	drawCorrespondenceMatrix: function(rows, cols, correspondences) {
		
		//$features = {rows: [type: refmap,  name: $ref_map_name, lgs: [lg1, lg2, ...]}, 
       	//             cols: {type: compmap,  name: $comp_map_name, lgs: [lg1, lg2, ...]}, 
       	//             corresp: [[1,2,...],[2,3,...],...]}
       	
		// return if there is no map
		if ((rows === null ) || (cols === null ) || (correspondences === null )) { 
			return;
		}

		var margin = {top: 110, right: 0, bottom: 0, left: 120};
	    var width = 600;
	    var height = 900;

		var svg = d3.select("#select_fieldset_correspondence_matrix_svg")
			.append("svg")
		    .attr("class", "TripalMap")
	    	.attr("width", width)
	    	.attr("height", height);
			
		// correspondence matrix frame 
	    var cmFrame = svg.append("g")
        	.attr("id", "cmFrame")
        	.attr("transform", "translate("+ margin.left + "," + margin.top + ")")
        	.attr("visibility", "unhidden");

	    var options = { "data": correspondences, "labelsCol" : cols['lgs'],  "mapNameCol": cols['name'],
        		"labelsRow": rows['lgs'],"mapNameRow": rows['name'], "start_color" : '#ffffff',"end_color" : '#3498db'};
        
	    drawMatrix(options, cmFrame);
        
        function drawMatrix(options, svg) {
        	
        	var data = options.data;
        	var labelsDataCol = options.labelsCol;
        	var mapNameCol = options.mapNameCol;
        	var labelsDataRow = options.labelsRow;
        	var mapNameRow = options.mapNameRow;
        	var startColor = options.start_color;
        	var endColor = options.end_color;
        	
        	if(!data){
        		throw new Error('Please pass data');
        	}

        	if(!Array.isArray(data) || !data.length || !Array.isArray(data[0])){
        		throw new Error('It should be a 2-D array');
        	}

        	var maxValue = d3.max(data, function(layer) { return d3.max(layer, function(d) { return d; }); });
        	var minValue = d3.min(data, function(layer) { return d3.min(layer, function(d) { return d; }); });

        	var numrows = data.length;
        	var numcols = data[0].length;
        	var cellsize = 45;
           	var width = numcols * cellsize;
        	var height = numrows * cellsize;
 
        	var background = svg.append("rect")
        		.style("stroke", "black")
        		.style("stroke-width", "2px")
        		.attr({"width": width,"height": height});

        	var x = d3.scale.ordinal()
				.domain(d3.range(numcols))
				.rangeBands([0, width]);

        	var y = d3.scale.ordinal()
        		.domain(d3.range(numrows))
        		.rangeBands([0, height]);

        	var colorMap = d3.scaleLinear()
        		.domain([minValue,maxValue])
        		.range([startColor, endColor]);

        	var row = svg.selectAll(".row")
        		.data(data)
        		.enter().append("g")
        		.attr({"class": "row","transform": function(d, i) { return "translate(0," + y(i) + ")"; }});

        	var cell = row.selectAll(".cell")
        		.data(function(d) { return d; })
        		.enter().append("g")
        		.attr({"class": "cell","transform": function(d, i) { return "translate(" + x(i) + ", 0)"; }});

        	cell.append('rect')
        		.attr({"width": x.rangeBand(),"height": y.rangeBand()})
        		.style("stroke-width", 0);

        	cell.append("text")
        		.attr({"dy": ".32em","x": x.rangeBand() / 2, "y": y.rangeBand() / 2, "text-anchor": "middle"})
        		.style("fill", function(d, i) { return d >= maxValue/2 ? 'white' : 'black'; })
        		.text(function(d, i) { return d; });

        	row.selectAll(".cell")
        		.data(function(d, i) { return data[i]; })
        		.style("fill", colorMap);

        	var labels = svg.append('g')
        		.attr('class', "labels");

        	// Columns
        	var columnMapLabel = labels.selectAll(".column-map-label")
				.data([{"x": 0, "y":0}])
				.enter().append("g")
				.append("text")			
 				.attr({"class": "column-map-label","font-size": "1.2em"})
 				.text(mapNameCol);

        	var colMapTextWidth = columnMapLabel.node().getBBox().width;
        	columnMapLabel.attr({"transform":"translate(" + (width / 2 - (colMapTextWidth / 2))+ ", -80)"});
        	
        	var columnLabels = labels.selectAll(".column-label")
        		.data(labelsDataCol)
        		.enter().append("g")
        		.attr({"class": "column-label","transform": function(d, i) { 
        			return "translate(" + x(i) + "," + -20 + ")"; }});
        	
        	columnLabels.append("line")
        		.style("stroke", "black")
        		.style("stroke-width", "1px")
        		.attr({"x1": x.rangeBand() / 2,"x2": x.rangeBand() / 2,"y1": 18,"y2": 20});

        	columnLabels.append("text")
        		.attr({"x": 0, "y": y.rangeBand() / 2,"dy": ".82em","text-anchor": "start", "transform": "rotate(290)"})
        		.text(function(d, i) { return d; });

        			
        	// Rows
        	var rowMapLabel = labels.selectAll(".row-map-label")
				.data([{"x": 0, "y":0}])
				.enter().append("g")
					.attr({"class":"row-map"})
					.attr({"transform":"rotate(270)"})
					.append("text")			
					.attr({"class": "row-map-label","font-size": "1.2em"})
					.text(mapNameRow);

        	var rowMapTextWidth = rowMapLabel.node().getBBox().width;
        	var rowTrans = "translate( " + (-(height / 2 + (rowMapTextWidth / 2))) + ",-80)";
        	rowMapLabel.attr({"transform":rowTrans});
        	
        	var rowLabels = labels.selectAll(".row-label")
        		.data(labelsDataRow)
        		.enter().append("g")
        		.attr({"class": "row-label","transform": function(d, i) { return "translate(" + 0 + "," + y(i) + ")"; }});

        	rowLabels.append("line")
        		.style("stroke", "black")
        		.style("stroke-width", "1px")
        		.attr({"x1": 0,"x2": -2,"y1": y.rangeBand() / 2,"y2": y.rangeBand() / 2});

        	rowLabels.append("text")
        		.attr({"x": -8,"y": y.rangeBand() / 2,"dy": ".32em","text-anchor": "end"})
        		.text(function(d, i) { return d; });
        	
        	}
	},
	
};


