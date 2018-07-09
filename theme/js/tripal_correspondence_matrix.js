
/* 
 * File: tripal_correspondence_matrix.js
 * Obtain settings from MapViewer PHP form, call linkageGroup draw to display selected chromosomes.
 */


(function($) {
  Drupal.behaviors.tripal_mapCorrespondenceMatrixBehavior = {
    attach: function (context, settings) {
    	
    	$('#select_fieldset_correspondence_matrix').once('select_fieldset_correspondence_matrix', function() {

    	var rows =				Drupal.settings.mapCorrespondenceMatrixJS.rows;                   	
       	var cols =				Drupal.settings.mapCorrespondenceMatrixJS.cols;
       	var correspondences =	Drupal.settings.mapCorrespondenceMatrixJS.correspondences;

        var container =  "#select_fieldset_correspondence_matrix";
    	$(container).before('<div></div><div id ="select_fieldset_correspondence_matrix_svg" width="100%"></div>');

    	correspondenceMatrix.drawCorrespondenceMatrix(rows, cols, correspondences);

    });
    }
  };
})(jQuery);

