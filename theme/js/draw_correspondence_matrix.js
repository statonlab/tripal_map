////////////////////////////////////////////////////////////
// Display correspondence matrix with D3.js

var correspondenceMatrix = correspondenceMatrix || {};

correspondenceMatrix = {  
		
	drawCorrespondenceMatrix: function(rows, cols, correspondences) {

		// return if there is no map
		if ((rows === null ) || (cols === null )) { 
			return;
		}

    	if(!correspondences) {
    		throw new Error('Please pass correspondences data');
    	}

    	if(!Array.isArray(correspondences) || !correspondences.length || !Array.isArray(correspondences[0])) {
    		throw new Error('Correspondences should be a 2-D array');
    	}

    	//$features = {rows: [type: refmap,  name: $ref_map_name, lgs: [lg1, lg2, ...]}, 
       	//             cols: {type: compmap,  name: $comp_map_name, lgs: [lg1, lg2, ...]}, 
       	//             corresp: [[1,2,...],[2,3,...],...]}
	    var svgMaxWidth = 1100;
	    var svgMaxHeight = 600;

    	var numrows = correspondences.length;
    	var numcols = correspondences[0].length;
    	var cellsize = 45;
       	var rectWidth = numcols * cellsize;
    	var rectHeight = numrows * cellsize;
    	var margin = {"x": 0, "y": 0};

    	var x = "";
    	var y = "";
    	if (tripalMap.d3VersionFour()) {
    		x = d3.scaleBand()
				.domain(d3.range(numcols))
				.range([0, rectWidth]);

    		y = d3.scaleBand()
    			.domain(d3.range(numrows))
    			.range([0, rectHeight]);
   
    	}
    	else {
    		x = d3.scale.ordinal()
				.domain(d3.range(numcols))
				.rangeBands([0, rectWidth]);

    		y = d3.scale.ordinal()
				.domain(d3.range(numrows))
				.rangeBands([0, rectHeight]);
    	}
	
		// If the exact mapName string occurs in the linkage group name, filter it out to reduce duplication
    	var labelsColLink = cols['lgs'];
    	var labelsRowLink = rows['lgs'];
		cols['lgs'] = cols['lgs'].map(x => tripalMap.lGNameReduction(x, cols['name']));
		rows['lgs'] = rows['lgs'].map(x => tripalMap.lGNameReduction(x, rows['name']));

	    var options = { 
	    	"data": correspondences, "labelsCol": cols['lgs'], "labelsColLink": labelsColLink, "mapNameCol": cols['name'], 
	    	"mapIdCol": cols['feature_id'], "labelsRow": rows['lgs'], "labelsRowLink": labelsRowLink, "mapNameRow": rows['name'],
	    	"mapIdRow": rows['feature_id'], "start_color" : '#ffffff', "end_color" : '#3498db', 
	    	"rectWidth": rectWidth,	"rectHeight": rectHeight, "xScale": x, "yScale": y};

	    var svgFieldset = "#select_fieldset_correspondence_matrix_svg";
	    d3.select(svgFieldset).selectAll("svg").remove(); 
	    var svg = d3.select(svgFieldset)
	    	.append("svg").attr("class", "TripalMap").attr("width", svgMaxWidth).attr("height", svgMaxHeight);

	    var labelDim = correspondenceMatrix.calculateLabelDimensions(svg, options, x, y, rectWidth, rectHeight);
	    var matrixWidth = margin.x + rectWidth + labelDim.x;
	    var matrixHeight = margin.y + rectHeight + labelDim.y;
	    
		if ((matrixWidth > svgMaxWidth) || (matrixHeight > svgMaxHeight)) {
			var cmScroll = "cm_scroll";
		    d3.select(svgFieldset).selectAll("svg").remove(); 
		    d3.select(svgFieldset).append("div").attr("class", "TripalMap").attr("id", cmScroll);
		    svg = d3.select("#"+cmScroll)
				.append("svg").attr("class", "TripalMap").attr("width", matrixWidth + 50).attr("height", matrixHeight + 50);
		}

		var pngFileName = 'Correspondence Matrix_' + options["mapNameCol"] + ' vs ' + options["mapNameRow"] + '.png'; 
		var stp = svg.append("svg:image");
		stp.attr("xlink:href", Drupal.settings.baseUrl+"/"+Drupal.settings.tripal_map.modulePath+"/theme/images/save_as_png.png")
		.attr('width', 35)
		.attr('height', 25)
		.attr("transform", "translate(" + 0 + "," + margin.y +")")
		.attr("id", "save_to_png")
		.on("click", function(d) { tripalMap.onSaveToPngClick(svg, pngFileName);})
		.on("mouseover", function(d) { tripalMap.onSaveToPngMouseOver(svg);})
		.on("mouseout", function(d) { tripalMap.onSaveToPngMouseOut(svg);});
		
		// correspondence matrix frame 
    	var stpn = stp.node().getBoundingClientRect();
    	var transY = stpn.height;

	    var cmFrame = svg.append("g");
        	cmFrame.attr("id", "cmFrame")
        	.attr("transform", "translate("+ margin.x + "," + (margin.y + transY) + ")") // position below the download-to-png icon
        	.attr("visibility", "unhidden");

	    var labels = cmFrame.append('g')
			.attr('class', "labels");

	    var colMapLabelHeight = correspondenceMatrix.drawColLabels(labels, options.mapNameCol, options.mapIdCol, options.labelsCol, x, y, rectWidth);
	    var rowMapLabelWidth = correspondenceMatrix.drawRowLabels(labels, options.mapNameRow, options.mapIdRow, options.labelsRow, y, rectHeight);
	    var translateX = rowMapLabelWidth; 
	    var translateY = colMapLabelHeight;
		labels.select(".column-labels").attr("transform", "translate("+ translateX + "," + 0 + ")");
		labels.select(".row-labels").attr("transform", "translate("+ 0 + "," + translateY + ")");

	    correspondenceMatrix.drawMatrix(options, cmFrame, translateX, translateY);
	},        
    
	calculateLabelDimensions: function(svg, options, x, y, rectWidth, rectHeight) {

		var labels = svg.append("g")
			.attr("class", "labels");
	    var colMapLabelHeight = correspondenceMatrix.drawColLabels(labels, options.mapNameCol, options.mapIdCol, options.labelsCol, x, y, rectWidth);
	    var rowMapLabelWidth = correspondenceMatrix.drawRowLabels(labels, options.mapNameRow, options.mapIdRow, options.labelsRow, y, rectHeight);
	    svg.selectAll(".labels").remove();
	    
		var labelDim = {"x": rowMapLabelWidth, "y": colMapLabelHeight};
		
		return labelDim;
	},
	
	drawColLabels: function(svg, mapNameCol, mapIdCol, labelsDataCol, x, y, width) {
	
		var columnLabels = svg.append("g")
			.attr("class","column-labels");
		
		var columnMapLabel = columnLabels.append("g")
			.attr("class","column-map-label")
			.append("a")	
			.attr("xlink:href", Drupal.settings.baseUrl + "/mapviewer/" + mapIdCol)
			.append("text")
			.attr("font-size", "1.2em")
			.text(mapNameCol);
		
		var mapLabelBuf = 20;
		var colMapTextWidth = columnMapLabel.node().getBBox().width;
		var colMapTextHeight = columnMapLabel.node().getBBox().height + mapLabelBuf;
		
    	// Columns
    	var columnLabels = columnLabels.append("g")
    		.attr("class", "column-cell-labels")
    		.selectAll(".column-cell-label")
    		.data(labelsDataCol)
    		.enter().append("g")
    			.attr("class", "column-cell-label")
    			.attr("transform", function(d, i) { return "translate(" + x(i) + "," + 0 + ")"; });
    	
    	xrange = "";
    	yrange = "";
    	if (tripalMap.d3VersionFour()) {
    		xrange = x.bandwidth();
    		yrange = y.bandwidth();
    	}
    	else {
    		xrange = x.rangeBand();
    		yrange = y.rangeBand();
    	}
    	columnLabels.append("line")
    		.style("stroke", "black")
    		.style("stroke-width", "1px")
    		.attr("x1", xrange / 2).attr( "x2", xrange / 2).attr("y1", 18).attr("y2", 20);
    		
    	columnLabels.append("text")
    		.attr("x", 0)
    		.attr("y", yrange / 2)
    		.attr("dy", ".82em")
    		.attr("text-anchor", "start")
    		.attr("transform", "rotate(290)")
    		.text(function(d, i) { return d; });

		var colLabelsHeight = svg.select(".column-cell-labels").node().getBBox().height;
		var colLabelsWidth = svg.select(".column-cell-labels").node().getBBox().width;

		columnMapLabel.attr("transform","translate(" + (width / 2 - (colMapTextWidth / 2))+ ", "+ (colMapTextHeight - mapLabelBuf) +")");
		svg.selectAll(".column-cell-labels").attr("transform","translate( 0, "+ (colLabelsHeight + mapLabelBuf) +")");

		return colLabelsHeight + colMapTextHeight;
	},

	drawRowLabels: function(svg, mapNameRow, mapIdRow, labelsDataRow, y, height) {

		var rowLabels = svg.append("g")
		.attr("class","row-labels");

		var rowMapLabel = rowLabels.append("g");
			rowMapLabel.attr("class", "row-map-label")
			.attr("transform","rotate(270)");
			
		var rowMapLabela = rowMapLabel.append("a");	
			rowMapLabela.attr("xlink:href", Drupal.settings.baseUrl + "/mapviewer/" + mapIdRow);
			
		var rowMapLabelText = rowMapLabela.append("text");
			rowMapLabelText.attr("font-size", "1.2em")
			.text(mapNameRow);
		
		var mapLabelBuf = 20;
		var rowMapTextWidth = rowMapLabelText.node().getBBox().width;
		var rowMapTextHeight = rowMapLabelText.node().getBBox().height + mapLabelBuf;
		
    	// Rows
    	var rowLabels = rowLabels.append("g")
    		.attr("class", "row-cell-labels")
    		.selectAll("row-cell-label")
    		.data(labelsDataRow)
    		.enter().append("g")
    			.attr("class", "row-cell-label")
    			.attr("transform", function(d, i) { return "translate(" + 0 + "," + y(i) + ")"; });

    	yrange = "";
    	if (tripalMap.d3VersionFour()) {
    		yrange = y.bandwidth();
    	}
    	else {
    		yrange = y.rangeBand();
    	}

    	rowLabels.append("line")
    		.style("stroke", "black")
    		.style("stroke-width", "1px")
    		.attr("x1", 0)
    		.attr( "x2", -2)
    		.attr( "y1", yrange / 2)
    		.attr( "y2", yrange / 2);

    	rowLabels.append("text")
    		.attr("x", -8)
    		.attr( "y", yrange / 2)
    		.attr( "dy", ".32em")
    		.attr( "text-anchor", "end")
    		.text(function(d, i) { return d; });

    	var rowLabelsWidth = svg.select(".row-cell-labels").node().getBBox().width;
    	var rowLabelsHeight = svg.select(".row-cell-labels").node().getBBox().height;

    	rowMapLabelText.attr("transform", "translate( " + (-(height / 2 + (rowMapTextWidth / 2))) + ", " + (rowMapTextHeight - mapLabelBuf) + ")");
    	svg.selectAll(".row-cell-labels")
    		.attr("transform", "translate( " + (rowLabelsWidth + rowMapTextHeight) + ", " + 0 + ")");
    	
    	return  rowLabelsWidth + rowMapTextHeight; // translated sideways, so offset is the horizontal (height)
	},

	drawMatrix: function(options, svg, translateX, translateY) {

		var data = options.data;
		var mapIdRow = options.mapIdRow;
		var mapIdCol = options.mapIdCol;
		var labelsCol = options.labelsCol;
		var labelsRow = options.labelsRow;
		var labelsColLink = options.labelsColLink;
		var labelsRowLink = options.labelsRowLink;
    	var startColor = options.start_color;
    	var endColor = options.end_color;
    	var width = options.rectWidth;
    	var height = options.rectHeight;
    	var x = options.xScale;
    	var y = options.yScale;
    	var numrows = data.length;
    	var numcols = data[0].length;

    	var maxValue = d3.max(data, function(layer) { return d3.max(layer, function(d) { return d; }); });
    	var minValue = d3.min(data, function(layer) { return d3.min(layer, function(d) { return d; }); });

    	var colorMap = d3.scaleLinear()
    		.domain([minValue,maxValue])
    		.range([startColor, endColor]);

    	var matrix = svg.append("g")
    		.attr("class","matrix")
    		.attr("transform", "translate("+ translateX + "," + translateY + ")");
    	
    	matrix.append("rect")
			.style("stroke", "black")
			.style("stroke-width", "2px")
			.attr("width", width)
			.attr("height", height);

    	var rows = matrix.append("g")
    		.attr("class", "rows")
    		.selectAll("row")
    		.data(data)
    		.enter().append("g")
    			.attr("class", "row")
    			.attr("transform", function(d, i) { return "translate(0," + y(i) + ")"; });

    	var cells = rows.selectAll(".cell")
    		.data(function(d) { return d; })
    		.enter().append("g")
    		.attr("class", "cell")
    		.attr("transform", function(d, i) { return "translate(" + x(i) + ", 0)"; });


    	// use encodeURI rather than encodeURIComponent as the latter replaces forward slashes with %2F which confuses Drupal menu_hook
    	var a = cells.append("a");
    	if (tripalMap.d3VersionFour()) {
    		var row = -1;
        	a.attr("xlink:href", function(d, col) { 
        		if (col == 0) {
        			row += 1;
        		}
        		var alink = Drupal.settings.baseUrl + "/mapviewer_comparison/" + mapIdRow +"/" 
        			+ encodeURI(labelsRowLink[row].toString().replace(/\//g, "_forwardslash_")) + "/" + mapIdCol + "/"
        			+ encodeURI(labelsColLink[col].toString().replace(/\//g, "_forwardslash_"));
        		
        		return alink;
        	});
    	}
    	else {
    		a.attr("xlink:href", function(d, col, row) { 
			return Drupal.settings.baseUrl + "/mapviewer_comparison/" + mapIdRow +"/" 
				+ encodeURI(labelsRowLink[row].toString().replace(/\//g, "_forwardslash_")) + "/" + mapIdCol + "/"
				+ encodeURI(labelsColLink[col].toString().replace(/\//g, "_forwardslash_")) });
    	}
    	xrange = "";
    	yrange = "";
    	if (tripalMap.d3VersionFour()) {
    		xrange = x.bandwidth();
    		yrange = y.bandwidth();
    	}
    	else {
    		xrange = x.rangeBand();
    		yrange = y.rangeBand();
    	}
    
    	var g = a.append("g");
    	g.append('rect')
    		.attr("width", xrange)
    		.attr("height", yrange)
    		.style("stroke-width", 0);
    	
    	g.append("text")
    		.attr("dy", ".32em")
    		.attr("x", xrange / 2)
    		.attr( "y", yrange / 2)
    		.attr( "text-anchor", "middle")
    		.style("fill", function(d, i) { return d >= maxValue/2 ? 'white' : 'black'; })
    		.text(function(d, i) { return d; });
			
    	rows.selectAll(".cell")
    		.data(function(d, i) { return data[i]; })
    		.style("fill", colorMap);

	},
	
};


