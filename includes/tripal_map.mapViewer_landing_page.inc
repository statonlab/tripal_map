<?php

/**
 * @file
 *	 Create form that displays the MapViewer landing page
 *	 Displays the select organism, map and linkage group toolbars and links to the correspondences matrix
 * 
 * @ingroup tripal_map
 */


/**
 * Implements hook_form().
 *
 * function: tripal_map_mapviewer_landing_pageform
 *   When displaying a landing page we need a form.  This function creates the form 
 *   used for this. Called by any reference to the /MapViewer page URL 
 *   
 * @return 
 *   A form array for the MapViewer Landing Page form.
 *    
 * @ingroup tripal_map_includes
 */
function tripal_map_mapviewer_landing_page_form($form, &$form_state) {

  $form['landing_page'] = array(
    '#type' => 'container',
    '#attributes' => array(
      'id' => array('landing_page'), // the js looks for this id to draw the map
      'class' => array("TripalMap")),
    '#collapsible' => FALSE,
  );
  
  $form['landing_page']['intro'] = array(
    // Brief description of what MapViewer is.
    '#markup' => '<div id="mapviewer_intro">
      <p>MapViewer is a graphical tool for viewing and comparing genetic maps. It includes dynamically scrollable maps, correspondence
       matrices, dot plots, links to details about map features, and exporting functionality.  It was developed by the MainLab at Washington State
       University and is available for download for use in other Tripal databases.  </p></div>',/* Functionality:
      1. interfaces show all linkage groups of a map, with zoom into specific regions of a linkage group,
      2. comparisons of maps that share the same markers are displayed.
      */
  );
 
  // display the toolbar to View Map
  $select_options = tripal_map_get_select_list_options();
  $featuremap_id = NULL;
  $linkage_group = NULL;
  tripal_map_form_landing_page_map_bar_init($form, $form_state, $select_options, $featuremap_id, $linkage_group);
  tripal_map_form_add_landing_page_map_bar($form, $form_state, $select_options);
  
  
  $form['landing_page']['usage'] = array(
    '#type' => 'container',
    '#attributes' => array(
    'id' => array('usage'), // the js looks for this id to draw the map
    'class' => array("TripalMap")),
    '#collapsible' => FALSE,
  );

  $video_tut_link = variable_get('tripal_map_video_tutorial_link', t(''));
  $text_tut_link = variable_get('tripal_map_text_tutorial_link', t(''));
  
  // obtain the sample map tutorial link
  $sample_map_tut_link = variable_get('tripal_map_sample_map_tutorial_link', t(''));
  $sample_map_id = "";
  if ($sample_map_tut_link != '') {
    if (substr($sample_map_tut_link, 0, 4) == "http") {
      // if the sample map path is a fully formed path ex. "https://.." leave it as is  
    }
    else {
      // a partial/relative path is provided, extract the map identifier
      $sample_map_id = preg_replace('/[^0-9]/', '', $sample_map_tut_link);
      $sample_map_tut_link = tripal_map_lookup_featuremap_page($sample_map_id);
    }
  }
  else {
    // the sample map path is unspecified by the user. Obtain the first map from the mview
    $sql = "
      SELECT map_id FROM {tripal_map_genetic_markers_mview}
      UNION
      SELECT map_id FROM {tripal_map_qtl_and_mtl_mview}
    ";
   
    $results = chado_query($sql);
    $pos = $results->fetchObject();
    $sample_map_id = $pos->map_id;
    $sample_map_tut_link = tripal_map_lookup_featuremap_page($sample_map_id);
   
  }

  // Create the Correspondence Matrix link
  $ref_map = tripal_map_get_elem($form_state['storage'], 'maps_key');
  $comp_map = 0;
  $map_org = $select_options['map_org'];
  $ak = array_keys($map_org);
  if (!empty($ak)) {
      $comp_map = $ak[0];
      if ((count($ak) > 1) && ($comp_map == $ref_map)) {
          $comp_map = $ak[1];
      }
  }
  $correspondence_matrix_url = "/correspondence_matrix/".$ref_map."/".$comp_map;
  
  // Create the Dot Plot link
  $dotplot_url = variable_get('tripal_map_sample_dotplot_tutorial_link', t('')); 
  if ($dotplot_url == '') {
    // The default dot plot is unspecified by the user. Obtain the first map/lg comparison
    $ref_lg_id = reset($map_org[$ref_map]['linkage_groups']);
    $select_options_comp = tripal_map_create_comparison_options($select_options, $ref_map, $ref_lg_id);
    $comp_org = key(tripal_map_get_elem($select_options_comp, 'options_org'));
    $comp_map = key($select_options_comp['options_map'][$comp_org]);
    $comp_lg_id = key($select_options_comp['options_chr'][$comp_org][$comp_map]);
    $dotplot_url = "/dotplot/".$ref_map."/".$ref_lg_id."/".$comp_map."/".$comp_lg_id;
  }
  else {
    if (substr($dotplot_url, 0, 4) == "http") {
      // if the sample dot plot path is a fully formed path ex. "https://.." leave it as is
    }
    else {
      $dotplot_url =  '/dotplot' . $dotplot_url;
    }
  }
  $example_export_url = variable_get('tripal_map_example_exported_figures_tutorial_link', t('')); //'/MapViewer_Exports';
  
  $form['landing_page']['usage']['title'] = array(
    '#markup' => '<div><h3>Learn more about MapViewer</h3>
    <ul><li id="video_tutorial"><a href="' . $video_tut_link . '" target="_blank">Video tutorial</a></li>
    <li id="user_manual"><a href="' . $text_tut_link . '" target="_blank">User Manual</a></li>
    <li id="sample_map"><a href="' . $sample_map_tut_link . '" target="_blank">View sample map</a></li>
    <li id="sample_correspondence_matrix"><a href="' . $correspondence_matrix_url . '" target="_blank">View example correspondence matrix</a></li>
    <li id="sample_dotplot"><a href="' . $dotplot_url . '" target="_blank">View example dot plot</a></li>
    <li id="sample_exported_figures"><a href="' . $example_export_url . '" target="_blank">View example exported figures</a></li>
    </ul></div>',
  );
  
  return $form;
  
}


/**
 * function: tripal_map_form_landing_page_map_bar_init
 *	 Initialize the reference toolbar, using trigger values if available.
 *
 * @param $form
 *   The form to be initialized
 *
 * @param $form_state
 *   The current state of the form. The values and storage that were submitted
 *
 *  @param $select_options
 *  @param $featuremap_id
 *  @param $linkage_group
 *
 * @return
 *	 The form array for the MapViewer form.
 *
 * @ingroup tripal_map_includes
 */
// assigns the $features array based on the featuremap id
function tripal_map_form_landing_page_map_bar_init( &$form, &$form_state, $select_options, &$featuremap_id, &$linkage_group) {

	tripal_map_init_organism_selectors_from_storage($form_state, $select_options, $featuremap_id, $linkage_group);
	
	$map_org = $select_options['map_org'];
	$options_org = $select_options['options_org'];
	$options_map = $select_options['options_map'];
	$options_chr = $select_options['options_chr'];

	$fs_storage =& $form_state['storage'];
	$org_key =& $fs_storage['org_key'];
	$maps_key =& $fs_storage['maps_key'];
	$chr_key =& $fs_storage['chr_key'];

	tripal_map_update_storage($form_state, 'maps', $options_map[$org_key], FALSE);
	tripal_map_update_storage($form_state, 'chr', $options_chr[$org_key][$maps_key], FALSE);

	$selector_toolbar = "landingpage_map_bar";
	$fs_selector_names = array("orgkey" => "org_key", "maps" => "maps", "mapskey" => "maps_key",
			"chr" => "chr", "chrkey" => "chr_key");
	$form_trigger_selectors = array(  "select_organism" => "select_organism", "select_map" => "select_map",
			"select_chromosome" =>"select_chromosome");

	tripal_map_trigger_update_storage_from_selectors($selector_toolbar, $fs_selector_names,
			$form_trigger_selectors, $select_options, $form_state);
	
	$form['landing_page']['toolbar']['map_select_frame']['select_organism']['#default_value'] = $org_key;
	$form['landing_page']['toolbar']['map_select_frame']['map_chr']['select_map']['#options'] = $fs_storage['maps'];
	$form['landing_page']['toolbar']['map_select_frame']['map_chr']['select_map']['#default_value'] = $maps_key;
	$form['landing_page']['toolbar']['map_select_frame']['map_chr']['select_chromosome']['#options'] = $fs_storage['chr'];
	$form['landing_page']['toolbar']['map_select_frame']['map_chr']['select_chromosome']['#default_value'] = $chr_key;
	return $form;

}


/**
 * function: tripal_map_form_add_landing_page_map_bar
 *	 Create the reference toolbar with selectors for organism, map and chromosome
 *
 * @param $form
 *   The form to be initialized
 *
 * @param $form_state
 *   The current state of the form. The values and storage that were submitted
 *   
 * @param $select_options
 *
 * @return
 *	 The form array for the MapViewer form.
 *
 * @ingroup tripal_map_includes
 */
function tripal_map_form_add_landing_page_map_bar(&$form, &$form_state, $select_options) {

  $map_org = $select_options['map_org'];
  $options_org = $select_options['options_org'];
  $options_map = $select_options['options_map'];
  $options_chr = $select_options['options_chr'];

  $fs_storage =& $form_state['storage'];
  $org_key =& $fs_storage['org_key'];
  $maps_key =& $fs_storage['maps_key'];
  $chr_key =& $fs_storage['chr_key'];
	
  $form['landing_page']['toolbar'] = array(
    '#type' => 'container',
    '#prefix' => '<div id="select_organism" class="TripalMap"><h3 class="reference">
    Select A Map To Start Using MapViewer</h3>',
    '#suffix' => '</div>',
    '#collapsible' => FALSE,
  );
  
  $form['landing_page']['toolbar']['info'] = array(
      '#markup' => '<div id= "mapviewer_toolbar_info">
      <p>When the "Submit" button is clicked, a new page for that map will open 
      and on that page a second linkage group for comparison can be selected.</p></div>',
  );
  

  $form['landing_page']['toolbar']['map_select_frame'] = array(
    '#type' => 'container',
    '#prefix' => '<div id="select_map_select_frame" class="TripalMap">',
    '#suffix' => '</div>',
    '#collapsible' => FALSE,
  );
	
  $form['landing_page']['toolbar']['map_select_frame']['select_organism'] = array(
    '#type' => 'select',
    '#title' => t('Species'),
    '#ajax' => array(
      'wrapper' => 'show-map-chr-landingpage-form', 
      'callback' => 'tripal_map_show_map_chr_landingpage_form_ajax_callback',
      'method' => 'replace',
        'progress' => array('type' => 'throbber'),
  ),
  '#options' => $options_org,
  '#multiple' => FALSE,
  '#default_value' => $org_key,
  );

  $form['landing_page']['toolbar']['map_select_frame']['map_chr'] = array(
    '#type' => 'container',
    '#prefix' => '<div id="map_chr' . '" class="form-item">',
    '#suffix' => '</div>',
    '#validated' => TRUE,
  );

  $form['landing_page']['toolbar']['map_select_frame']['map_chr']['select_map'] = array(
    '#type' => 'select',
    '#title' => t('Map'),
    '#prefix' => '<div id="select_map' . '" class="form-item">',
    '#suffix' => '</div>',
    '#ajax' => array(
      'wrapper' => 'show-map-chr-landingpage-form', // 'select_chromosome',
      'callback' => 'tripal_map_show_map_chr_landingpage_form_ajax_callback',
      'progress' => array('type' => 'throbber'),
      'method' => 'replace',
    ),
    '#options' => $fs_storage['maps'],
    '#multiple' => FALSE,
    '#default_value' => $maps_key,
    '#needs_validation' => FALSE,
  );

  $form['landing_page']['toolbar']['map_select_frame']['map_chr']['select_chromosome'] = array(
    '#type' => 'select',
    '#title' => t('Linkage Group'),
    '#prefix' => '<div id="select_chromosome' . '" class="form-item">',
    '#suffix' => '</div>',
    '#ajax' => array(
      'wrapper' => 'show-map-chr-landingpage-form',
      'callback' => 'tripal_map_show_map_chr_landingpage_form_ajax_callback',
      'progress' => array('type' => 'throbber'),
      'method' => 'replace',
    ),
    '#options' => $fs_storage['chr'],
    '#multiple' => FALSE,
    '#default_value' => $chr_key,
  );

  $featuremap_id = tripal_map_get_elem($fs_storage, 'maps_key');
  $lg_id = tripal_map_get_elem($fs_storage, 'chr_key');
  $lg_name = tripal_map_get_lg_name_from_id($form_state, $org_key, $featuremap_id, $lg_id);
  $mapviewer_url = "/mapviewer/".$featuremap_id."/".urlencode($lg_name);
	
  $form['landing_page']['toolbar']['map_select_frame']['button_map_select_landingpage-submit'] = array(
    '#type' => 'container', 
    '#value' => t('Submit'),
    '#name' => ('Submit_Reference'),
    '#attributes' => array(
      'id' => 'map_select_landingpage-submit',
     ),
     '#prefix' =>
       '<div id="show_mapviewer" class="form-item">
       <label class="mapviewer"><a href= ' . $mapviewer_url . '
       target="_blank" title="MapViewer">Submit  </a></label>',
  	 '#suffix' => '</div>',
	
  );

	 
  $form['#prefix'] = '<div id="show-map-chr-landingpage-form">';
  $form['#suffix'] = '</div>';

  return $form;

}

