<?php

/**
 * @file
 *	 Create map comparison toolbars for select organism, map and linkage group for the MapViewer forms.
 *
 * @ingroup tripal_map
 */


/**
 * function: tripal_map_init_organism_comparison_selectors_from_storage
 *	 Initialize form state storage variables for comparison selectors
 *
 * @param $form_state
 *   The current state of the form. The values and storage that were submitted
 *
 * @param $featuremap_id
 * @param $linkage_group
 *
 * @ingroup tripal_map_includes
 */
function tripal_map_init_organism_comparison_selectors_from_storage(&$form_state, $select_options_corres, $featuremap_id_comparison, $linkage_group_comparison) {

  $map_org_comp = $select_options_corres['map_org'];

  $fs_storage =& $form_state['storage'];
  if (!(array_key_exists('storage', $form_state))) {
    // the storage key does not exist in form state
    $fs_storage = array();
  }

  if (!is_array($fs_storage)) {
    // the form state is initialized to null, by Drupal. Assign it to type array.
    $fs_storage = array();
  }

  $submit = FALSE;
  if ((tripal_map_get_form_trigger($form_state) == 'Submit_Comparison') || (tripal_map_get_form_trigger($form_state) == 'Submit_Reference')) {
    $submit = TRUE;
  }
  /*if (!$submit) {
    tripal_map_update_storage($form_state, 'org_comp_all', tripal_map_get_elem($select_options_corres, 'options_org'), TRUE);
    tripal_map_update_storage($form_state, 'maps_comp_all', tripal_map_get_elem($select_options_corres, 'options_map'), TRUE);
    tripal_map_update_storage($form_state, 'chr_comp_all', tripal_map_get_elem($select_options_corres, 'options_chr'), TRUE);
  }*/
  // by default, do not show comparison. set to false if flag does not exist.
  tripal_map_update_storage($form_state, 'show_comp', FALSE, FALSE);
  
  // take the first organism as default
  if (!$submit) {
    tripal_map_update_storage($form_state, 'org_compkey', key(tripal_map_get_elem($select_options_corres, 'options_org')), TRUE);
    if ((isset($featuremap_id_comparison) && strlen($featuremap_id_comparison) > 0 ) ) {
      // if featuremap id is available, lookup the associated organism and use that as default
      tripal_map_update_storage($form_state, 'org_compkey', tripal_map_get_organism_id($map_org_comp, $featuremap_id_comparison), TRUE);
    }
  }

  // use the first map as default
  if (!$submit) {
    tripal_map_update_storage($form_state, 'maps_compkey', key($select_options_corres['options_map'][$fs_storage['org_compkey']]), TRUE);
    if (isset($featuremap_id_comparison) && strlen($featuremap_id_comparison)) {
      // set the default map to the id passed by featuremap_id_comparison
      tripal_map_update_storage($form_state, 'maps_compkey', $featuremap_id_comparison, TRUE);
    }
  }

  if (!$submit) {
    // use the first chromosome as a default
    tripal_map_update_storage($form_state, 'chr_compkey', key($select_options_corres['options_chr'][$fs_storage['org_compkey']][$fs_storage['maps_compkey']]), TRUE);
    if ((isset($featuremap_id_comparison) && strlen($featuremap_id_comparison) > 0 ) &&
      (isset($linkage_group_comparison) && strlen($linkage_group_comparison) > 0 )) {
      tripal_map_update_storage($form_state, 'chr_compkey', get_chr_id($map_org_comp, $featuremap_id_comparison, $linkage_group_comparison), TRUE);
    }
  }
}


/**
 * function: tripal_map_form_organism_comparison_bar_init
 *	 Initialize data structures for the comparison toolbar
 *
 * @param $form
 *   The form to be initialized
 *
 * @param $form_state
 *   The current state of the form. The values and storage that were submitted
 *
 * @param $featuremap_id
 * @param $linkage_group
 * @param $features_comparison
 * @param $show_comp
 *
 * @return
 *	 The form array for the MapViewer form.
 *
 * @ingroup tripal_map_includes
 */
function tripal_map_form_organism_comparison_bar_init( &$form, &$form_state, $select_options_corres, 
      &$featuremap_id_comparison, &$linkage_group_comparison, &$features_comparison, &$show_comp) {

  $fs_storage =& $form_state['storage'];

  // check if there was a trigger in the reference toolbar
  $ref_trigger = FALSE; 
  if ((tripal_map_get_form_trigger($form_state) == 'select_organism') || 
    (tripal_map_get_form_trigger($form_state) == 'select_map') ||
    (tripal_map_get_form_trigger($form_state) == 'select_chromosome')) {
    $ref_trigger = TRUE;
  }
  
  // by default pick the first element of each select list
  $org_id = key(tripal_map_get_elem($select_options_corres, 'options_org'));
  $featuremap_id_comparison = key($select_options_corres['options_map'][$org_id]);
  if (array_key_exists('maps_compkey', $form_state['storage']) && !$ref_trigger) {
     $featuremap_id_comparison = $fs_storage['maps_compkey'];
  } 
  
  // get the linkage group name from id, in case not already present in storage
  //$linkage_group_key = "";
  
  // initialize the values for select lists for organism, map and linkage group, if none are
  // currently set. None are set if form_state storage for these is uninitialized.
  tripal_map_init_organism_comparison_selectors_from_storage($form_state, $select_options_corres, 
  		$featuremap_id_comparison, $linkage_group_comparison);

  $org_compkey =& $fs_storage['org_compkey'];
  $maps_compkey =& $fs_storage['maps_compkey'];
  $chr_compkey =& $fs_storage['chr_compkey'];
  $show_comp =& $fs_storage['show_comp'];
  
  $submit = FALSE;
  if ((tripal_map_get_form_trigger($form_state) == 'Submit_Comparison') && (tripal_map_get_form_trigger($form_state) == 'Submit_Reference')) {
    $submit = TRUE;
  }
  if (!$submit) {
    // if the reference map is updated, then set the comparison map to new defaults correspondingly 
    tripal_map_update_storage($form_state, 'org_comp', tripal_map_get_elem($select_options_corres, 'options_org'), TRUE);
    tripal_map_update_storage($form_state, 'maps_comp', $select_options_corres['options_map'][$org_compkey], TRUE);
    tripal_map_update_storage($form_state, 'chr_comp', $select_options_corres['options_chr'][$org_compkey][$maps_compkey], TRUE);
  }

  if (tripal_map_get_form_trigger($form_state) == 'select_organism_comparison') {
    // the organism comparison selection item has changed
    tripal_map_update_storage($form_state, 'org_compkey', $form_state['triggering_element']['#value'], TRUE);
    tripal_map_update_storage($form_state, 'maps_comp', $select_options_corres['options_map'][$org_compkey], TRUE);
    tripal_map_update_storage($form_state, 'maps_compkey', key($select_options_corres['options_map'][$org_compkey]), TRUE);
    tripal_map_update_storage($form_state, 'chr_comp', $select_options_corres['options_chr'][$org_compkey][$maps_compkey], TRUE);
    tripal_map_update_storage($form_state, 'chr_compkey', key($select_options_corres['options_chr'][$org_compkey][$maps_compkey]), TRUE);
  }
  if (tripal_map_get_form_trigger($form_state) == 'select_map_comparison') {
    // the map comparison selection item has changed
    tripal_map_update_storage($form_state, 'maps_compkey', $form_state['triggering_element']['#value'], TRUE);
    tripal_map_update_storage($form_state, 'chr_comp', $select_options_corres['options_chr'][$org_compkey][$maps_compkey], TRUE);
    tripal_map_update_storage($form_state, 'chr_compkey', key($select_options_corres['options_chr'][$org_compkey][$maps_compkey]), TRUE);
  }

  if (tripal_map_get_form_trigger($form_state) == 'select_chromosome_comparison') {
    // the chromosome comparison selection item has changed
    tripal_map_update_storage($form_state, 'chr_compkey', $form_state['triggering_element']['#value'], TRUE);
  }

  if (tripal_map_get_form_trigger($form_state) == 'show_comparison') {
    //save the show comparison setting in form state storage, so it is accessible to JS
    tripal_map_update_storage($form_state, 'show_comp', $form_state['values']['show_comparison'], TRUE);
  }
  
  $form['organism_comparison']['select_organism_comparison']['#default_value'] = $org_compkey;
  $form['organism_comparison']['map_chr_comparison']['select_map_comparison']['#options'] = $fs_storage['maps_comp'];
  $form['organism_comparison']['map_chr_comparison']['select_map_comparison']['#default_value'] = $maps_compkey;
  $form['organism_comparison']['map_chr_comparison']['select_chromosome_comparison']['#options'] = $fs_storage['chr_comp'];
  $form['organism_comparison']['map_chr_comparison']['select_chromosome_comparison']['#default_value'] = $chr_compkey;
  $form['organism_comparison']['#disabled'] = !$show_comp;

  $linkage_group_comparison = tripal_map_get_lg_name_from_id($form_state, $org_compkey, $maps_compkey, $chr_compkey);
  tripal_map_update_storage($form_state, 'linkage_group_comparison', $linkage_group_comparison, TRUE);
  tripal_map_update_storage($form_state, 'featuremap_id_comparison', $maps_compkey, TRUE);

  // get marker data based on featuremap_id and linkage group selected from drop lists
  if (array_key_exists('featuremap_id_comparison', $fs_storage)) {
    $featuremap_id_comparison = $fs_storage['featuremap_id_comparison'];
  }
  if (array_key_exists('linkage_group_comparison', $fs_storage)) {
    $linkage_group_comparison = $fs_storage['linkage_group_comparison'];
  }
  
  $organism_id_comparison = $org_compkey;

  $features_comparison = tripal_map_create_features($featuremap_id_comparison, $chr_compkey, $organism_id_comparison);

  return $form;
}


/**
 * function: tripal_map_form_add_organism_comparison_bar
 *	 Add the comparison toolbar to the MapViewer form
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
function tripal_map_form_add_organism_comparison_bar(&$form, &$form_state, $select_options) {

  $fs_storage =& $form_state['storage'];
  
  // Add the dot plot to the top of the Control panel, now that ref map and chr is available
  $dotplot_icon = '/' . drupal_get_path('module', 'tripal_map') . '/theme/images/dotplot.png';
  $ref_map = tripal_map_get_elem($fs_storage, 'maps_key');
  $ref_chr = tripal_map_get_elem($fs_storage, 'chr_key');
  $comp_map = tripal_map_get_elem($fs_storage, 'maps_compkey');
  $comp_chr = tripal_map_get_elem($fs_storage, 'chr_compkey');
  $dotplot_url = "/dotplot/".$ref_map."/".$ref_chr."/".$comp_map."/".$comp_chr;
  $form['dotplot'] = array(
    '#type' => 'container',
    '#prefix' => 
  	  '<div id="show_dot_plot" class="TripalMap">
      <label class="dotplot">Dot Plot  <a href= ' . $dotplot_url . '
  	  target="_blank" title="Dot Plot"><img id="dotplot_img" src="' . $dotplot_icon . 
  	 '" align="top"></a></label>',
    '#suffix' => '</div>',
    '#collapsible' => FALSE,
  );
  
  // Create the Correspondence Matrix link
  $ref_map = tripal_map_get_elem($fs_storage, 'maps_key');
  $comp_map = tripal_map_get_elem($fs_storage, 'maps_compkey');
  $correspondence_matrix_url = "/correspondence_matrix/".$ref_map."/".$comp_map;
  $form['main_comparison'] = array(
    '#type' => 'container',
    '#prefix' => '<div id="show_main_comparison" class="TripalMap">
  	  <label class="reference">Select Comparison Map: </label>'
      .'<a id="correspondence_matrix_link" href= '.$correspondence_matrix_url.' target="_blank"
  	  title="Correspondence Matrix">(View Correspondence Matrix)</a></label>',
  	'#suffix' => '</div>',
    '#collapsible' => FALSE,
  );

  $form['main_comparison']['mc_frame'] = array(
    '#type' => 'container',
    '#prefix' => '<div id="show_mc_frame" class="TripalMap">',
    '#suffix' => '</div>',
    '#collapsible' => FALSE,
  );
  
  $form['main_comparison']['mc_frame']['comparison'] = array(
    '#type' => 'container',
    '#prefix' => '<div id="show_comparison_map" class="TripalMap">',
    '#suffix' => '</div>',
    '#collapsible' => FALSE,
  );

  $form['main_comparison']['mc_frame']['comparison']['show_comparison'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show comparison map'),
    '#default_value' => tripal_map_get_elem($fs_storage, 'show_comp'), 
    '#ajax' => array(
      'wrapper' => 'show-map-chr-form', 
      'callback' => 'tripal_map_show_map_chr_form_ajax_callback',
      'method' => 'replace',
      'progress' => array('type' => 'throbber'),
    ),
  );
    
  $form['main_comparison']['mc_frame']['organism_comparison'] = array(
    '#type' => 'container',
    '#prefix' => '<div id="select_organism_comparison" class="TripalMap">',
    '#suffix' => '</div>',
    '#collapsible' => FALSE,
    '#disabled' => !tripal_map_get_elem($fs_storage, 'show_comp')
  );

  $form['main_comparison']['mc_frame']['organism_comparison']['select_organism_comparison'] = array(
    '#type' => 'select',
    '#title' => t('Species'),
    '#ajax' => array(
      'wrapper' => 'show-map-chr-form', 
      'callback' => 'tripal_map_show_map_chr_form_ajax_callback',
      'method' => 'replace',
      'progress' => array('type' => 'throbber'),
    ),
    '#options' => tripal_map_get_elem($select_options, 'options_org'),
    '#multiple' => FALSE,
    '#default_value' => tripal_map_get_elem($fs_storage, 'org_compkey'),
  );

  $form['main_comparison']['mc_frame']['organism_comparison']['map_chr_comparison'] = array(
    '#type' => 'container',
    '#prefix' => '<div id="map_chr_comparison' . '" class="form-item">',
    '#suffix' => '</div>',
    '#validated' => TRUE,
  );

  $form['main_comparison']['mc_frame']['organism_comparison']['map_chr_comparison']['select_map_comparison'] = array(
    '#type' => 'select',
    '#title' => t('Map'),
    '#prefix' => '<div id="select_map_comparison' . '" class="form-item">',
    '#suffix' => '</div>',
    '#ajax' => array(
      'wrapper' => 'show-map-chr-form', // 'select_chromosome',
      'callback' => 'tripal_map_show_map_chr_form_ajax_callback',
      'progress' => array('type' => 'throbber'),
      'method' => 'replace',
    ),
    '#options' => tripal_map_get_elem($fs_storage, 'maps_comp'),
    '#multiple' => FALSE,
    '#default_value' => tripal_map_get_elem($fs_storage, 'maps_compkey'),
    '#needs_validation' => FALSE,
  );

  $form['main_comparison']['mc_frame']['organism_comparison']['map_chr_comparison']['select_chromosome_comparison'] = array(
    '#type' => 'select',
    '#title' => t('Linkage Group'),
    '#prefix' => '<div id="select_chromosome_comparison' . '" class="form-item">',
    '#suffix' => '</div>',
    '#ajax' => array(
      'wrapper' => 'show-map-chr-form', 
      'callback' => 'tripal_map_show_map_chr_form_ajax_callback',
      'progress' => array('type' => 'throbber'),
      'method' => 'replace',
    ),
    '#options' => tripal_map_get_elem($fs_storage, 'chr_comp'),
    '#multiple' => FALSE,
    '#default_value' => tripal_map_get_elem($fs_storage, 'chr_compkey'),

  );

  $form['main_comparison']['mc_frame']['organism_comparison']['button_chromosome_comparison_mv-submit'] = array(
    '#type' => 'submit', 
    '#value' => t('Submit'),
    '#name' => ('Submit_Comparison'),
    '#attributes' => array(
      'id' => 'chromosome_comparison_mv-submit'),
    '#prefix' => '<div class="chromosome_comparison_mv-submit-button">',
    '#suffix' => '</div>',
    '#submit' => array('tripal_map_submit_rebuild_form_state'),
    '#after_build' => array('tripal_map_draw_mapViewer'),
  );

  return $form;
}


/**
 * function: tripal_map_update_drupal_add_js_show_comparison_params
 *	 Pass the show comparison settings to JS
 *
 * @param $show_comparison
 *
 * @ingroup tripal_map_includes
 */
function tripal_map_update_drupal_add_js_show_comparison_params($show_comparison) {

  // pass flag for whether to display comparison linkage group or not
  $js_setting = "mapViewerDisplayComparisonJS";
  drupal_add_js(array($js_setting => array('show_comparison' => $show_comparison)), 'setting');

}

