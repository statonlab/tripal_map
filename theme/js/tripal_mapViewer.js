
/* 
 * File: tripal_mapViewer.js
 * Obtain settings from MapViewer PHP form, call linkageGroup draw to display selected chromosomes.
 */

(function($) {
	Drupal.behaviors.tripal_mapViewerBehavior = {
	attach: function (context, settings) {
    	
    	$('#select_fieldset_mapViewer').once('select_fieldset_mapViewer', function() {
    	
    	// Reference chromosome	
    	var geneticFeatures =         Drupal.settings.mapViewerDisplayJS.mapViewer_genetic_features;
    	var mapName =                 Drupal.settings.mapViewerDisplayJS.reference_map_name;        
       	var linkageGroupName =        Drupal.settings.mapViewerDisplayJS.reference_linkage_group;        
           	
       	// Comparison chromosome
    	var geneticFeaturesComparison = Drupal.settings.mapViewerDisplayComparisonJS.mapViewer_genetic_features_comparison;
    	var mapComparisonName =         Drupal.settings.mapViewerDisplayComparisonJS.comparison_map_name;        
       	var linkageGroupComparisonName =    Drupal.settings.mapViewerDisplayComparisonJS.comparison_linkage_group;        
       	var showComparison = 			Drupal.settings.mapViewerDisplayComparisonJS.show_comparison;
       
    	var markerTypeDisplayStates = Drupal.settings.mapViewerDisplayJS.marker_type_display_states;
     	var markerTypeColorMap =      Drupal.settings.mapViewerDisplayJS.marker_type_color_map;
        
    	var showRuler = 0;
    	if ($("#show_ruler_mapViewer:checked").val() !== undefined) {
        	   showRuler = 1;
		}

		var showMarkerPos = 0;
    	if ($("#marker_pos_mapViewer:checked").val() !== undefined) {
        	   showMarkerPos = 1;
		}
    	
    	var container =  "#select_fieldset_mapViewer";
    	$(container).before('<div></div><div id ="select_fieldset_mapViewer_svg" width="100%"></div>');

     	// be very careful when creating a unique list of markers as some have the same name, but different position. 
    	var uniqueMarkers = [];
    	for(var key in geneticFeatures) {
    		uniqueMarkers.push(geneticFeatures[key]);
    	}
    	var strUniqueMarkers = JSON.stringify(uniqueMarkers);
    	var uniqueMarkersComparison = [];
    	for(var key in geneticFeaturesComparison) {
    		uniqueMarkersComparison.push(geneticFeaturesComparison[key]);
    	}
    	var strUniqueMarkersComparison = JSON.stringify(uniqueMarkersComparison);
    	
       	var strMarkerTypeDisplayStates = "";
		if (Object.keys(markerTypeDisplayStates).length > 0) {
    		strMarkerTypeDisplayStates = JSON.stringify(markerTypeDisplayStates);
    	}
		
		var linkageGroupId = "lg";
		var mapId = uniqueMarkers[0].featuremap_id;
		var linkageGroupComparisonId = "lgComp";
		var mapComparisonId = uniqueMarkersComparison[0].featuremap_id;
		var markerData = new configMapViewer.MarkerData(strMarkerTypeDisplayStates, markerTypeColorMap);
	    markerData.addLinkageGroup(uniqueMarkers, linkageGroupName, mapName, linkageGroupId, mapId, mapViewer.OrientationEnum.LEFT);
		if (showComparison) {
	    markerData.addLinkageGroup(uniqueMarkersComparison, linkageGroupComparisonName, mapComparisonName, linkageGroupComparisonId, mapComparisonId, mapViewer.OrientationEnum.RIGHT);
		}
		markerData.findCorrespondences();
	    var show = {Ruler: showRuler, MarkerPos: showMarkerPos, Comparison: showComparison};
	    
	    // get the svg width
		var svgFieldset = "#select_fieldset_mapViewer_svg";
	    d3.select(svgFieldset).selectAll("svg").remove(); 
	    var svgHeight = 600;
	    var svgMaxWidth = 1100;
	    var svg = d3.select(svgFieldset)
	    	.append("svg").attr("class", "TripalMap").attr("width", svgMaxWidth).attr("height", svgHeight);
     
		var chrFrameReference = new mapViewer.ChrFrame(mapViewer.OrientationEnum.LEFT, show, mapName, mapId, linkageGroupName, markerData);
		var chrFrameRefWidth = chrFrameReference.calculateWidth(svg);
		var chrFrameCompWidth = 0;
		var chrFrameComparison = "";
		if (showComparison) {
			chrFrameComparison = new mapViewer.ChrFrame(mapViewer.OrientationEnum.RIGHT, show, mapComparisonName, mapComparisonId, linkageGroupComparisonName, markerData);
			chrFrameCompWidth = chrFrameComparison.calculateWidth(svg);
		}

		// if a scrollbar is required, nest the svg in the div for scrolling
		var svgWidth = chrFrameRefWidth + chrFrameCompWidth + 200;
		var mvScroll = "mv_scroll";
	    d3.select(svgFieldset).selectAll("svg").remove(); 
		if (svgWidth > svgMaxWidth) {
	    	d3.select(svgFieldset)
				.append("div").attr("class", "TripalMap").attr("id", mvScroll);
			svg = d3.select("#"+mvScroll)
				.append("svg").attr("class", "TripalMap").attr("width", svgWidth).attr("height", svgHeight);
		}
		else {
			svg = d3.select(svgFieldset)
	            .append("svg").attr("class", "TripalMap").attr("width", svgMaxWidth).attr("height", svgHeight);
	    }
	    
		var pngFileName = 'MapViewer_' + mapName + '_' + linkageGroupName + '_vs_' + mapComparisonName + '_' + linkageGroupComparisonName + '.png'; 
		var stp = svg.append("svg:image");
		stp.attr("xlink:href", Drupal.settings.baseUrl+"/sites/all/modules/tripal_map/theme/images/save_as_png.png")
		.attr('width', 35)
		.attr('height', 25)
		.attr("transform", "translate(" + 0 + "," + 0 +")")
		.attr("id", "save_to_png")
		.on("click", function(d) { tripalMap.onSaveToPngClick(svg, pngFileName);})
		.on("mouseover", function(d) { tripalMap.onSaveToPngMouseOver(svg);})
		.on("mouseout", function(d) { tripalMap.onSaveToPngMouseOut(svg);});

		var transY = stp.node().getBoundingClientRect().height;

	    // Reference linkage group
	    var translateX = 0;
	    var translateY = 35 + transY;
		chrFrameReference.setTranslate(translateX, translateY);
		svg = chrFrameReference.draw(svg);
		translateX = chrFrameRefWidth + 15; // take the statically calculated width based on full data set instead of zoomed section //svg.node().getBBox().width;

		if (showComparison) {
			// Comparison linkage group
			chrFrameComparison.setTranslate(translateX, translateY);
			svg = chrFrameComparison.draw(svg);
		}
		
		// draw the legend (including marker types that are configured as hidden) but only for the selected linkage groups	
		var legend = new mapViewer.Legend(markerData);
		var buf = svgHeight - (chrFrameReference.translateY + chrFrameReference.height + 15 /*approx legend height*/);
		translateY = chrFrameReference.translateY + chrFrameReference.height + buf;
		legend.setTranslate(0, translateY);
		svg = legend.draw(svg);
			
    });
    }
  };
})(jQuery);

