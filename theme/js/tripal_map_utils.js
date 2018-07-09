
////////////////////////////////////////////////////////////
// Utility and helper functions for tripalMap draw routines

var tripalMap = tripalMap || {};

    tripalMap = {

	findCorrespondences: function(data) {
	    
		//output format: {"UBC190":[{"linkageGroup":"LG1", "position":"97.5"},{"linkageGroup":"LG2","position":"85.3"}],
		//                "UBC43":[{"linkageGroup":"LG1","position":"97.5","linkageGroup":"LG2","position":"14"}]}
		
		// hash the markers for the reference chromosome
		var refChrMarkerHash = {};
		data.forEach(function(d) {
			if (!(d.genetic_marker_locus_name in refChrMarkerHash)) {
				refChrMarkerHash[d.genetic_marker_locus_name] = [];
			}
			var markerPos = "";
			if (d.genetic_marker_type == "QTL") {
				if (d.hasOwnProperty("marker_qtl_peak")) {
					markerPos = d.marker_qtl_peak;
				}
				if (d.hasOwnProperty("marker_start_pos")) {
					markerPos = d.marker_start_pos;
				}
			}
			else {
				markerPos = d.marker_start_pos; 
			}
			refChrMarkerHash[d.genetic_marker_locus_name].push({"linkageGroup":d.linkage_group,"position":markerPos});
		});  
		
		// keep only the markers that appear in more than one linkage group.
		var markerCorrespondences = {};
		for (var k in refChrMarkerHash) {
			if (refChrMarkerHash.hasOwnProperty(k)) {
				if (refChrMarkerHash[k].length > 1) {
					markerCorrespondences[k] = refChrMarkerHash[k];
				}
			}
		}
		
		return markerCorrespondences;
	},

		
	orderFeaturemapToc: function( featuremapId) {
		var sidebar = document.getElementById('chado_featuremap-tripal-toc-pane');
		if (sidebar) {
			
			// Order the TOC items
			var div = sidebar.getElementsByTagName('div');
			var ordered_items = [];
			var index = 2;

			for (var i = 0; i < div.length; i ++) {

				var as = div[i].getElementsByTagName('a');
				if (as.length === 0) {
					continue;
				}
				var title = as[0].innerHTML.trim();
				if (title.toLowerCase() === 'view map'){
					// set the map viewer pane tab to link to a clean mapviewer window
					var mapviewerHref = Drupal.settings.baseUrl+"/mapviewer/"+featuremapId;
					as[0].setAttribute('href', mapviewerHref);
					as[0].setAttribute('target', '_blank');
					if(div[i].hasAttribute('class')){
						// override class="tripal_toc_list_item", to avoid interference from tripal js 
						div[i].setAttribute('class', 'map-viewer-item');
					}
					if(as[0].hasAttribute('class')){
						// override class="tripal_toc_list_item_link", to avoid interference from tripal js 
						as[0].setAttribute('class', 'map-viewer-link');
					}
				}
				
				
				// Data
				if (title.toLowerCase() === 'map overview' ) {
					ordered_items [0] = jQuery(div[i]).clone();
				} else if (title.toLowerCase() === 'view map') {
					ordered_items [1] = jQuery(div[i]).clone();
				}		
				else{
					ordered_items [index] = jQuery(div[i]).clone();
					index = index + 1;
				}				
			}
			
			// Output
			sidebar.innerHTML = '';			
			//jQuery(sidebar).prepend('<i>Data</i><br>');
			for (var i = 0; i < ordered_items.length; i ++) {
				if (typeof(ordered_items[i]) !== 'undefined') {
					jQuery(sidebar).append(ordered_items[i]);
				}
			}
		}
	},
    		
    		
    // function: getMarkerTypesColors
    // Input: associative array of all genetic markers and their property values
    // Output: array of all genetic marker types
    getMarkerTypes: function (markers) {

		var markerTypes = [];
		markers.forEach(function(feature){
			if (!(markerTypes.indexOf(feature.genetic_marker_type) > -1)){
				markerTypes.push(feature.genetic_marker_type);
			}
		});

		return markerTypes;
		
	},

	d3VersionFour: function(){
		return false;
	},
	
	convertTextColor: function(color) {
		if (color === "#FFFF67") {
			// change yellow to a darker, more visible shade for text
			return "#8C96A8";
		} 
		else {
			return color; 
		}
	},	
	

	sanitizeChrUrlId: function(linkageGroup){
		var str = linkageGroup;
		//return encodeURIComponent(str);
		var res = str.replace(/\+/g, "%2B");
		return res;
	},


	wrap: function(text, width) {
		
		text.each(function() {
			var breakChars = ['/', '&', '-', ',', '.', '_',')', ']', '}', ':'];
		    var text = d3.select(this);
		    var textContent = text.text();
		    var spanContent = '';
		    
		    breakChars.forEach(function(char) {
		    	// Add a space after each break char for the function to use to determine line breaks
		    	textContent = textContent.replace(char, char + ' ');
		    });
	
		    var words = textContent.split(/\s+/).reverse();
		    var word = [];
		    var line = [];
		    var lineNumber = 0;
		    var lineHeight = text.style('line-height');
		    var x = text.node().getBBox().x; 
		    var y = text.attr('y');
		    var dy = parseFloat(text.attr('dy') || 0);
		    var tspan = text.text(null);
		    
		    while (word = words.pop()) {
		    	line.push(word);
		    	tspan.text(line.join(' '));
		    	if (tspan.node().getComputedTextLength() > width) {
		    		line.pop();
		    		spanContent = line.join(' ');
		    		breakChars.forEach(function(char) {
		    			// Remove spaces trailing breakChars that were added above
		    			spanContent = spanContent.replace(char + ' ', char);
		    		});
		    		tspan.text(spanContent);
		    		line = [word];
		    		tspan = text.append('tspan')
		        		.attr('x', x)
		        		.attr( 'dy', lineHeight)
		        		.text(word);
		    	}
		    }
	    });

	},
	
	lGNameReduction: function(linkageGroup, mapNameIn) {

		var linkageGroupName = linkageGroup;
		
		// test the linkageGroup name for map name duplicate inclusion
		var mapName = mapNameIn.replace(/['"]+/g, '');
		var mapNameIndex = linkageGroupName.indexOf(mapName); // -1 if never occurs
		if ((mapNameIndex > -1) && ((mapNameIndex + mapName.length) < linkageGroupName.length)){
			// If the exact mapName string occurs in the linkage group name, remove it
			linkageGroupName = linkageGroupName.substr( mapNameIndex + mapName.length+1, linkageGroupName.length - 1);
		}
		return linkageGroupName;
	},
	
	encodeHtmlIdAttrib: function(unsafe) {
		// remove all chars from the linkage group name that are not accepted in an Html id attribute
	    return unsafe.replace(/[^a-zA-Z0-9-_]/g, "_");
	 },

};
