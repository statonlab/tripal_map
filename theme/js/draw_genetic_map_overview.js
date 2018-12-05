////////////////////////////////////////////////////////////
// Display genetic map overview with D3.js
// Draw the glyphs for each chromosome of the map. Clicking on a glyph will 
// launch the MapViewer page displaying the corresponding chromosome

var geneticOverviewMap = geneticOverviewMap || {};

	geneticOverviewMap = {

	mapOverviewDraw: function(dataMarkers, displayConfig) {

	var dc = displayConfig;
	if (dc.markerTypeDisplayStates.length <= 0 ) {
		return;
	}
	
	// container holds linkage group glyphs
	var container = {topMargin: 100, rightMargin: 10, bottomMargin: 100, leftMargin: 50, width: 600, maxWidth: 900, 
			height: 177, text_height_adj: 4};
    container['color'] = {outline: "black"};
    container['lines'] = {markerWidth: 3, outlineWidth: 1, lineToLabelWidth: 1 };

    container['chr'] = {width: 18, bkgd: "#C89696", highlightBkgd: "#FFDCDC", height: container.height - container.topMargin - container.bottomMargin};
    var marginChr = {top: 85, right: 10, bottom: 100, left: 40}; 
    var glyphDistanceSeparation = 70;

    var chr = container['chr'];
    chr['totalWidth'] = 75;
    chr['curve'] = 	{radius: {x: 13, y: 18}};
    chr['offsets'] = {topLabel: {x: -165, y: -50}, vertical: 17};
    chr['marker_locus'] = {start: {x: .5 }, end: {x: chr.width - .5 }, labelOffset: {x: 2*chr.width}};
    chr['MTL'] = {start: {x: chr.marker_locus.start.x}, end: {x: chr.marker_locus.end.x}, labelOffset: {x: chr.marker_locus.labelOffset.x}};
    chr['QTL'] = {start: {x: 0}, end: {x: 0}, markerOffset: {x: -8 }, markerIncrOffset: {x: -4}, 
      orientation: "left", labelOffset: {x: chr.width/*-4*chr.width*/}};   
 
    // append a field for color, based on marker type and add marker start position, obtain number of linkage groups
	var markerTypeColors = dc.colorMap;
	
	// add linkage groups to array and sort by linkage group name key
    var data = (JSON.parse(dataMarkers));
	var linkageGroupsAr = [];
	data.forEach(function(d) {
		if (!(d.linkage_group)) {
			// the linkage group name is null, but data is still associated with it. assign it the string "null" to proceed.
			d.linkage_group = "Null";
		}
		if (!(linkageGroupsAr.indexOf(d.linkage_group) > -1)) {
			linkageGroupsAr.push(d.linkage_group);
		}
		
	});

	linkageGroupsAr.sort();

	// get linkage group label max height to adjust svg for linkage group label multi-line wrap
	var maxChrLabelVerticalHeight = 0;
	linkageGroupsAr.forEach( function(linkageGroup) {			

		// work around for CottonGen only
		linkageGroupName = tripalMap.lGNameReduction(linkageGroup, dc.mapName);

		var t = d3.select("#select_fieldset_genetic_map_overview").append("svg");
			t.append("text")
			.attr("class", "lgtext")
			.style("font-size", "1em").style("line-height", ".9em")
			.text(Drupal.t( '@linkageGroup', {'@linkageGroup': linkageGroupName}))	  
			.call(tripalMap.wrap, glyphDistanceSeparation);

		var lgText = t.selectAll(".lgtext");
		var textHeight = lgText.node().getBBox().height;

		if (textHeight > maxChrLabelVerticalHeight) {
			maxChrLabelVerticalHeight = textHeight;
		}	
		t.remove();
	});
	container.height = container.height + maxChrLabelVerticalHeight;

	// set the start, stop positions for all markers, and container height based on map type and max marker value
	data = setContainerHeight(data, dc, container);
	
	// setup svg container width and height 
	// assign dynamic container width and scrolling if number of linkage groups exceeds page width.
	// requires container.height, container.width and container.maxWidth to be preset
	var svg = setupSvgContainer(linkageGroupsAr.length, container);

	// add chromosome view to svg, containing map name
    var chrView = svg.append("g")
		.attr("transform", "translate(" + marginChr.left + "," + (marginChr.top /*+ transY*/) +")")
        .attr("visibility", "unhidden");

    var pngFileName = 'MapOverview_' + dc.mapName + '.png'; 
	var stp = chrView.append("svg:image");
	stp.attr("xlink:href", Drupal.settings.baseUrl+"/"+Drupal.settings.tripal_map.modulePath+"/theme/images/save_as_png.png")
		.attr('width', 32)
		.attr('height', 23)
		.attr("transform", "translate(" +  (-marginChr.left) + "," + (chr.offsets.topLabel.y + 6) +")")
		.attr("id", "save_to_png")
		.on("click", function(d) { tripalMap.onSaveToPngClick(svg, pngFileName);})
		.on("mouseover", function(d) { tripalMap.onSaveToPngMouseOver(svg);})
		.on("mouseout", function(d) { tripalMap.onSaveToPngMouseOut(svg);});

	var transY = stp.node().getBoundingClientRect().height;
	var transX = stp.node().getBoundingClientRect().width;

    chrView.append("text")
		.attr("id", "text-select").attr("dx", -marginChr.left + 7 + transX).attr("dy",(chr.offsets.topLabel.y + transY))
		.style("font-size", "1em")
		.text('Viewing '+dc.mapType+' map '+dc.mapName);	  

	// add one glyph per linkage group to the chromosome view, including the map name
	var count = 0;
	linkageGroupsAr.forEach( function(linkageGroup) {			

		var dataLG = [];
		data.forEach(function(d) {
			if (d.linkage_group === linkageGroup) {
				dataLG.push(d);
			}
		});

		if (dataLG.length > 0) {

			// Chromosome label above view 
			// Cottongen work around
			linkageGroupName = tripalMap.lGNameReduction(linkageGroup, dc.mapName);

			var dx = (count*glyphDistanceSeparation) + 5;
			var dy = chr.offsets.topLabel.y+ transY + 40;
			chrView.append("text")
				.attr("class", "lgtext").attr( "id", "text-select").attr("dx", dx).attr("dy", dy)
				.style("font-size", "1em").style("line-height", ".9em")
				.text(linkageGroupName)	  
				.call(tripalMap.wrap, glyphDistanceSeparation);
			
            // assigning marker positions must be done after each positional chromosome has chr.marker_locus.x assignment
			dataLG = processMarkers(dataLG);

			// set the chromosome height
			if (dc.mapType.toLowerCase() == "physical") {
				// this is a physical map, fix the height to a static value as linkage group heights can vary by 
				// several orders of magnitude within the same map, which does not render well
				container.chr.height = 100;
			}
			else {
				// otherwise draw the dynamic chromosome height
				container.chr.height = d3.max(dataLG.map(function(d) { return d.marker_pos_high; }));
			}
			y = d3.scaleLinear().range([0, container.chr.height]); 
			x = d3.scaleLinear().range([0, chr.width]); //width
			y.domain([0, d3.max(dataLG.map(function(d) { return d.marker_pos_high; }))]); // input domain
			
			var chrId = "chrView"+tripalMap.lGNameReduction(linkageGroup, dc.mapName); 
			var sanitizedChrId = tripalMap.encodeHtmlIdAttrib(chrId);
			xChrStartOffset = count*glyphDistanceSeparation + 5;
			yChrStartOffset = 2*chr.offsets.vertical + maxChrLabelVerticalHeight;
			chrView.append("rect")
				.data(dataLG)
				.attr("id", sanitizedChrId).attr("x", xChrStartOffset)
				.attr("y", chr.offsets.vertical + maxChrLabelVerticalHeight)
				.attr("rx", chr.curve.radius.x).attr("ry", chr.curve.radius.y) 
				.attr("width", chr.width).attr("height", container.chr.height + 2*chr.offsets.vertical)
				.style("fill", chr.bkgd).style("stroke", container.color.outline)
				.style("stroke-width", container.lines.outlineWidth)
				.on("mouseover", mouseover).on( "mouseout", mouseout).on("click", onclick);
	
			// draw the markers on the chromosome view
			chrView.selectAll("line.horizontal.chr")
				.data(dataLG)
				.enter().append("svg:line")
				.attr("x1", function(d) { return xChrStartOffset + d.chr.start.x;}) 
				.attr("y1", function(d) { return (y(d.chr.start.y) + yChrStartOffset);})
				.attr("x2", function(d) { return xChrStartOffset + d.chr.end.x;}) 
				.attr("y2", function(d) { return (y(d.chr.end.y) + yChrStartOffset);})
				.style("stroke", function(d) { return d.color; }).style("stroke-width", container.lines.markerWidth)
				.on("mouseover", mouseover).on( "mouseout", mouseout).on( "click", onclick);
				
		}
		count += 1;
		
	});

	// 	highlight the first linkage group glyph if linkage groups exist
	if (linkageGroupsAr.length > 0) {
		var chrId = tripalMap.lGNameReduction(linkageGroupsAr[0], dc.mapName);
		var sanitizedChrId = tripalMap.encodeHtmlIdAttrib(chrId); 
		chrView.selectAll("rect")
			.style("fill", chr.bkgd);	
		chrView.select("#chrView"+sanitizedChrId)
			.style("fill", chr.highlightBkgd);   
	}

	// set the start, stop positions for all markers, and container height based on map type and max marker value
	function setContainerHeight(data, dc, container) {
		
		data.forEach(function(d) {
			var peak_height = 2;
			if (d.hasOwnProperty("marker_start_pos")) {
				d.marker_start_pos = +parseFloat(Number(d.marker_start_pos).toFixed(10));
				if (!(d.hasOwnProperty("marker_stop_pos")) && (d.genetic_marker_type === "QTL")) {
					// if a QTL has a start but no stop position, add one as with qtl peak.
					d.marker_stop_pos = d.marker_start_pos + peak_height;
				}
			}
			if (d.hasOwnProperty("marker_qtl_peak")) {
				d.marker_qtl_peak = +parseFloat(Number(d.marker_qtl_peak).toFixed(10));
				// give the QTL peak a height. So peak becomes the start value, and peak+peak_height is the stop
				d.marker_stop_pos = d.marker_qtl_peak + peak_height;
			} 	
			if (d.hasOwnProperty("marker_stop_pos")) {
				d.marker_stop_pos = +parseFloat(Number(d.marker_stop_pos).toFixed(10));
			} 
	        d.chrEndY = (d.hasOwnProperty("marker_stop_pos")) ? d.marker_stop_pos : 
	        (d.hasOwnProperty("marker_qtl_peak")) ? d.marker_qtl_peak : d.marker_start_pos; 
		});
		
		if (dc.mapType.toLowerCase() == "physical") {
			// this is a physical map, so set height to a static value. These linkage group heights can vary by 
			// several orders of magnitude within the same map which impacts rendering
			container.height = container.height + 100;
		}
		else {
		    // adjust the container height based on longest chromosome
			var maxHeightChr = d3.max(data.map(function(d) { return d.chrEndY; }));	
		    container.height = container.height + maxHeightChr;
		}
		
		return data;
	}


	function setupSvgContainer(numLinkageGroups, container) {

		var svg = "";
		// assign dynamic container width if number of linkage groups exceeds page width.
		dynamicContainerWidth = numLinkageGroups*container.chr.totalWidth;
		if (container.width < dynamicContainerWidth) {
			container.width = dynamicContainerWidth;
		}
		
		// if a scrollbar is required, nest the svg in the div for scrolling
		var mo_glyph_scroll = "";
	    if (container.width > container.maxWidth) {
	    	mo_glyph_scroll = d3.select("#select_fieldset_genetic_map_overview")
				.append("div")
				.attr("id", "mo_glyph_scroll");

			svg = d3.select("#mo_glyph_scroll")
				.append("svg")
				.attr("class", "TripalMap")
				.attr("width", container.width).attr("height", container.height);
		}
		else {
	       svg = d3.select("#select_fieldset_genetic_map_overview")
	            .append("svg")
	            .attr("class", "TripalMap")
				.attr("width", container.width).attr("height", container.height);
	    }

	    return svg;
	}
	
	function processMarkers(dataLG) {

		var qtlYPos = [];
        var qtlCount = 0;

		var numQTLs = 0;
		dataLG.forEach(function(d) {
			if (d.genetic_marker_type === "QTL") {
				numQTLs += 1;
			}
			if (d.hasOwnProperty("marker_start_pos")) {
				d.marker_start_pos = +parseFloat(Number(d.marker_start_pos).toFixed(10));
			}
			if (d.hasOwnProperty("marker_qtl_peak")) {
				d.marker_qtl_peak = +parseFloat(Number(d.marker_qtl_peak).toFixed(10));
			} 	
			if (d.hasOwnProperty("marker_stop_pos")) {
				d.marker_stop_pos = +parseFloat(Number(d.marker_stop_pos).toFixed(10));
			} 
            d.chrStartY = (d.hasOwnProperty("marker_start_pos")) ? d.marker_start_pos : 
             (d.hasOwnProperty("marker_qtl_peak")) ? d.marker_qtl_peak : d.marker_stop_pos; 
		});
		dataLG.sort(function(a, b) { return a.chrStartY - b.chrStartY; });

		dataLG.forEach(function(d) {
			
			d['chr'] = {start: {}, end: {}};
			// determine the start and stop y loci positions in this order of precedence, depending on 
			// position data available for the marker 
			// start: start, qtl_peak, stop
			// stop: stop, qtl_peak, start
			d['chr']['start']['y'] = (d.hasOwnProperty("marker_start_pos")) ? d.marker_start_pos : 
				   				(d.hasOwnProperty("marker_qtl_peak")) ? d.marker_qtl_peak : d.marker_stop_pos; 
			d['chr']['end']['y'] = (d.hasOwnProperty("marker_stop_pos")) ? d.marker_stop_pos :
				   			d.hasOwnProperty("marker_qtl_peak") ? d.marker_qtl_peak : d.marker_start_pos;
			d.marker_pos_high = d.chr.end.y; 
			d.marker_pos = +d.marker_start_pos; 
			if (d.genetic_marker_type === "QTL") {

				// The marker is a QTL
	        	var curYPos = [];
	        	curYPos['start'] = d.chr.start.y;
	        	curYPos['end'] = d.chr.end.y;
	        	
	        	var qtlYPosArrayLen = qtlYPos.length; 
	        	if (qtlYPosArrayLen > 0) {

	        		// The Y end position of previous QTL less than Y start position of new QTL 
	        		// This QTL can fit in the same lane, so reset qtlCount to 0
	        		var prevYEndPos = qtlYPos[qtlYPosArrayLen - 1]['end'];
	 				if (prevYEndPos < curYPos.start) {
	 					qtlCount = 0;
	 				}
	 				// if this marker has the same position as the previous marker, also fit it in the 
	 				// same lane, superimposing it on the previous one.
	 				var prevYStartPos = qtlYPos[qtlYPosArrayLen - 1]['start'];
	 				if ((prevYStartPos === curYPos.start) && (prevYEndPos === curYPos.end)) {
	 					qtlCount = 0;
	 				}

	        	}
	        	qtlYPos.push(curYPos);
	        	qtlOffsetChrX = chr.QTL.markerOffset.x + qtlCount*chr.QTL.markerIncrOffset.x;
	        	d['chr']['start']['x'] = qtlOffsetChrX;
	        	d['chr']['end']['x'] = qtlOffsetChrX;
				qtlCount += 1;
				
			}
			else if ((d.genetic_marker_type === "heritable_phenotypic_marker")) {

				d['chr'] = {start: {x: chr.MTL.start.x }, end: {x: chr.MTL.end.x }, labelOffset: {x: chr.MTL.labelOffset.x } }; 	
			}
			else {

				d['chr'] = {start: {x: chr.marker_locus.start.x }, end: {x: chr.marker_locus.end.x }, 
								labelOffset: {x: chr.marker_locus.labelOffset.x }}; 					
			}
			// choose the start and stop y loci positions in this order of preference:
			// start: start, qtl_peak, stop; stop: stop, qtl_peak, start
			d['chr']['start']['y'] = (d.hasOwnProperty("marker_start_pos")) ? d.marker_start_pos : 
                           (d.hasOwnProperty("marker_qtl_peak")) ? d.marker_qtl_peak : d.marker_stop_pos; 
			d['chr']['end']['y'] = (d.hasOwnProperty("marker_stop_pos")) ? d.marker_stop_pos :
                           d.hasOwnProperty("marker_qtl_peak") ? d.marker_qtl_peak : d.marker_start_pos;
			d.marker_pos_high = d.chr.end.y; 
            d.marker_pos = +d.marker_start_pos; 
            if (d.genetic_marker_type in markerTypeColors) {
            // if the marker type exists in the colormap, assign the corresponding color
                d.color = markerTypeColors[d.genetic_marker_type];
            }
            else {
                // as a default set the marker color to black
                d.color = "black";
            }

		});

		return dataLG;
	}

	function mouseover() {
		svg.style("cursor", "pointer");
	}
	
	function mouseout() {
		svg.style("cursor", "default");

	}

	function onclick(d) {
		// escape the forward slash character, to allow Drupal menu_hook to interpret linkage group names containing forward slashes
		// as a single argument rather than tokenizing them into multiple args.
		res = d.linkage_group.toString().replace(/\//g, "_forwardslash_");
		// use encodeURI rather than encodeURIComponent as the latter replaces forward slashes with %2F which confuses Drupal menu_hook
		window.open(Drupal.settings.baseUrl+"/mapviewer/"+dc.featuremapId+"/"+encodeURI(res));		
				
		// highlight the clicked on linkage group glyph
		var chrId = tripalMap.lGNameReduction(d.linkage_group, dc.mapName); 
		var sanitizedChrId = tripalMap.encodeHtmlIdAttrib(chrId);
		chrView.selectAll("rect")
			.style("fill", chr.bkgd);		
		chrView.select( "#chrView"+sanitizedChrId)
			.style("fill", chr.highlightBkgd);

		return false; 
	}

},

};
