<?php

/**
 * @file
 *	 Creates field formatter class for the tripal map mapviewer overview field
 *
 * @ingroup tripal_map
 */

class tripal_map__mapviewer_overview_formatter extends ChadoFieldFormatter {

  // The default label for this field.
  public static $default_label = 'mapviewer_overview';

  // The list of field types for which this formatter is appropriate.
  public static $field_types = array('tripal_map__mapviewer_overview');

  // The list of default settings for this formatter.
  public static $default_settings = array('setting1' => 'default_value',);

  
  /**
   * Implements the hook_field_formatter_settings_form()
   *
   * function: settingsForm
   *   Provides the field's setting form appearing on the 'Manage Display' page of the content
   *   type admin page.
   *
   * @param $view_mode
   *   The view mode being configured.
   *
   * @param $form
   *   The (entire) configuration form array, which will usually have no use
   *   here.  Typically for reference only.
   *
   * @param &$form_state
   *   The form state of the (entire) configuration form.
   *
   * @return
   *   A Drupal Form array containing the settings form for this field.
   *
   * @ingroup tripal_map_includes
   */
  public function settingsForm($view_mode, $form, &$form_state) {

  }

  
  /**
   * Implements the hook_field_formatter_settings_summary()
   *
   * function: settingsSummary
   *   Provides a summary of the formatter settings appearing on the 'Manage Display' page of the content
   *   type admin page.
   *
   * @param $view_mode
   *   The view mode being configured.
   *
   * @return $summary
   *   A string description field settings summary
   */
  public function settingsSummary($view_mode) {
      
      return 'MapViewer Overview';
  }
  
  /**
   * Implements the hook_field_formatter_view()
   *
   * function: view
   *   Provides the display for the mapviewer overview field when viewed on the web page.
   *
   *  @param $element
   *  @param $entity_type
   *  @param $entity
   *  @param $langcode
   *  @param $items
   *  @param $display
   *
   *  @return $element
   *   A Drupal element array containing the rendered mapviewer overview form.
   */
  public function view(&$element, $entity_type, $entity, $langcode, $items, $display) {

    if ($entity->chado_table == 'organism') {
      $select = [0 => '--Please Select a Map--'];
      $select_add = $items[0]['select_choices'];
      if (empty($select_add)) {
        return;
      }
          
      // if only one featuremap, just use that
      if (count($select_add) === 1) {
        $form = drupal_get_form('tripal_map_genetic_map_overview_form', key($select_add));
        $content = drupal_render($form);
        $element[] = [
          '#type' => 'markup',
          '#markup' => $content,
        ];
        return;
      }
          
      // multiple maps for this organism, let user select.  Create a special form for that so we can have an AJAX select box
      $select= $select + $select_add;
      $form = drupal_get_form('tripal_map_organism_featuremap_selector', $select);
      $content = drupal_render($form);
      $element[] = [
        '#type' => 'markup',
        '#markup' => $content,
      ];
      return $element;
    }
      
    // case: featuremap
    $featuremap = $entity->chado_record;
  	$form = drupal_get_form('tripal_map_genetic_map_overview_form', $featuremap->featuremap_id);
    $content = drupal_render($form);
    $element[] = [
      '#type' => 'markup',
      '#markup' => $content,
    ];
    return;
  }
  
}