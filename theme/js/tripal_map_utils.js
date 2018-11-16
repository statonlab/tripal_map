
////////////////////////////////////////////////////////////
// Utility and helper functions for TripalMap draw routines

var tripalMap = tripalMap || {};

    tripalMap = {

    // Create Event Handlers for mouse
	onSaveToPngMouseOver: function(svg) {

		svg.style("cursor", "pointer");
    	var stpG = svg.append("g")
    		.attr("id", "save_to_png_text");
    	
    	var stp = d3.select("#save_to_png");
    	var stpn = stp.node().getBoundingClientRect();
    	var translateX = stpn.x - svg.node().getBoundingClientRect().x;
    	var translateY = stpn.y - svg.node().getBoundingClientRect().y + stpn.height + 5;

    	stpG.attr("transform", "translate("+ translateX + "," + translateY + ")");
    
    	var stpT = stpG.append("text");
    	stpT.text("Download as a png");

    	var stpTHeight = stpT.node().getBoundingClientRect().height;
    	var stpTWidth = stpT.node().getBoundingClientRect().width;
    	
    	var pad = 8;
    	var stpR = stpG.append("rect");
		stpR.style("fill","#6C7386")
		.attr("width", stpTWidth + pad)
		.attr("height", stpTHeight + pad);

		stpT.remove();
		stpG.append("text")
			.text("Download as a png")
			.style("fill","white")
			.attr("x", (pad/2))
			.attr("y", stpTHeight + (pad/2));

	},
	
	onSaveToPngMouseOut: function(svg) {
		svg.style("cursor", "default");
        
        // Select text by id and then remove
        d3.select("#save_to_png_text").remove(); // Remove text location
	},

	onSaveToPngClick: function(svg, pngFileName) { 
		
		var stp = d3.select("#save_to_png");
		var stpNode = stp.node();
		var stpParent = stp.node().parentNode;
		d3.select("#save_to_png_text").remove();
		stp.remove(); // Remove icon before saving png 
		
		var svgString = tripalMap.getSVGString(svg.node());

		var width = parseInt(svg.style("width"));
		var height = parseInt(svg.style("height"));
		
		tripalMap.svgString2Image( svgString, 2*width, 2*height, 'png', save ); // passes Blob and filesize String to the callback
		function save( dataBlob, filesize ) {
			saveAs( dataBlob, pngFileName ); // FileSaver.js function
		}
		
		stpParent.append(stpNode); // replace icon
	},

    		
	// Below are the functions that handle actual exporting:
	// getSVGString ( svgNode ) and svgString2Image( svgString, width, height, format, callback )
	getSVGString: function(svgNode) {
		svgNode.setAttribute('xlink', 'http://www.w3.org/1999/xlink');
		var cssStyleText = getCSSStyles( svgNode ); // this modifies the font size 
		appendCSS( cssStyleText, svgNode );

		var serializer = new XMLSerializer();
		var svgString = serializer.serializeToString(svgNode);
		// remove the node style attribute
		var style = document.getElementById("tripal_map_style");
		style.parentNode.removeChild(style);

		svgString = svgString.replace(/(\w+)?:?xlink=/g, 'xmlns:xlink='); // Fix root xlink without namespace
		svgString = svgString.replace(/NS\d+:href/g, 'xlink:href'); // Safari NS namespace fix

		return svgString;

		function getCSSStyles( parentElement ) {
			var selectorTextArr = [];

			// Add Parent element Id and Classes to the list
			selectorTextArr.push( '#'+parentElement.id );
			for (var c = 0; c < parentElement.classList.length; c++)
					if ( !contains('.'+parentElement.classList[c], selectorTextArr) )
						selectorTextArr.push( '.'+parentElement.classList[c] );

			// Add Children element Ids and Classes to the list
			var nodes = parentElement.getElementsByTagName("*");
			for (var i = 0; i < nodes.length; i++) {
				var id = nodes[i].id;
				if ( !contains('#'+id, selectorTextArr) )
					selectorTextArr.push( '#'+id );

				var classes = nodes[i].classList;
				for (var c = 0; c < classes.length; c++)
					if ( !contains('.'+classes[c], selectorTextArr) )
						selectorTextArr.push( '.'+classes[c] );
			}

			// Extract CSS Rules
			var extractedCSSText = "";
			for (var i = 0; i < document.styleSheets.length; i++) {
				var s = document.styleSheets[i];
				
				try {
				    if(!s.cssRules) continue;
				} catch( e ) {
			    		if(e.name !== 'SecurityError') throw e; // for Firefox
			    		continue;
			    	}

				var cssText = "";
				var cssRules = s.cssRules;
				for (var r = 0; r < cssRules.length; r++) {
					//if ( contains( cssRules[r].selectorText, selectorTextArr ) )
					if (cssRules[r].selectorText) {
						if (cssRules[r].selectorText == "svg.TripalMap") {
							extractedCSSText += "svg.TripalMap { font: .8em sans-serif; padding-left: 1em;}";
						}                
						else {
							extractedCSSText += cssRules[r].cssText;
						}
					}
				}
			}
			return extractedCSSText;

			function contains(str,arr) {
				return arr.indexOf( str ) === -1 ? false : true;
			}

		}

		function appendCSS( cssText, element ) {
			var styleElement = document.createElement("style");
			styleElement.setAttribute("type","text/css");
			styleElement.setAttribute("id", "tripal_map_style");
			styleElement.innerHTML = cssText;
			var refNode = element.hasChildNodes() ? element.children[0] : null;
			element.insertBefore( styleElement, refNode );
		}
	},


	svgString2Image: function( svgString, width, height, format, callback ) {

		var format = format ? format : 'png';
		var imgsrc = 'data:image/svg+xml;base64,'+ btoa( unescape( encodeURIComponent( svgString ) ) ); // Convert SVG string to data URL

		var canvas = document.createElement("canvas");
		var context = canvas.getContext("2d");

		canvas.width = width;
		canvas.height = height;

		var image = new Image();
		image.onload = function() {
			context.clearRect ( 0, 0, width, height );
			context.drawImage(image, 0, 0, width, height);
			
			  context.save();
			  context.globalCompositeOperation = 'destination-over';
			  context.fillStyle = 'white';
			  context.fillRect(0, 0, canvas.width, canvas.height);
			  // Restore the original context state previously saved
			  context.restore();
			
			canvas.toBlob( function(blob) {
				var filesize = Math.round( blob.length/1024 ) + ' KB';
				if ( callback ) callback( blob, filesize );
			});
		};

		image.src = imgsrc;

	}, 
		
 			
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
			refChrMarkerHash[d.genetic_marker_locus_name].push({"linkageGroup":d.linkage_group,"position":markerPos, "feature_id": d.feature_id});
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
				if (title.toLowerCase() === 'view map') {
					// set the map viewer pane tab to link to a clean mapviewer window
					var mapviewerHref = Drupal.settings.baseUrl+"/mapviewer/"+featuremapId;
					as[0].setAttribute('href', mapviewerHref);
					as[0].setAttribute('target', '_blank');
					if(div[i].hasAttribute('class')) {
						// override class="tripal_toc_list_item", to avoid interference from tripal js 
						div[i].setAttribute('class', 'map-viewer-item');
					}
					if(as[0].hasAttribute('class')) {
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
			for (var i = 0; i < ordered_items.length; i ++) {
				if (typeof(ordered_items[i]) !== 'undefined') {
					jQuery(sidebar).append(ordered_items[i]);
				}
			}
		}
	},
    		
 	d3VersionFour: function() {
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
	

	sanitizeChrUrlId: function(linkageGroup) {
		var str = linkageGroup;
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

		var linkageGroupName = linkageGroup.replace(/['"]+/g, '');
		
		// test the linkageGroup name for map name duplicate inclusion
		var mapName = mapNameIn.replace(/['"]+/g, '');
	
		var mapNameIndex = linkageGroupName.indexOf(mapName); // -1 if never occurs
		if ((mapNameIndex > -1) && ((mapNameIndex + mapName.length) < linkageGroupName.length)) {
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
