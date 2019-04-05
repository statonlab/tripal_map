<?php

/**
 * @file
 *   Creates field widget class for the tripal map marker map positions field
 *
 * @ingroup tripal_map
 */


class tripal_map__mapviewer_mappos_widget extends ChadoFieldWidget {

  // The default label for this field.
  public static $default_label = 'mapviewer_mappos';

  // The list of field types for which this formatter is appropriate.
  public static $field_types = array('tripal_map__mappos');

  
  /**
   * 
   * Implements the hook_field_widget_form()
   *
   * function: form
   *   Provides the form for editing the marker map positions field and is displayed when the entity 
   *   the field is attached to, is being modified.
   * 
   * @param &$widget
   * 
   * @param &$form
   *   The form structure where widgets are being attached to. This might be a
   *   full form structure, or a sub-element of a larger form.
   *   
   * @param &$form_state
   *   An associative array containing the current state of the form.
   *   
   * @param $langcode
   *   The language associated with $items.
   *   
   * @param $items
   *   Array of default values for this field.
   *   
   * @param $delta
   *   The order of this item in the array of subelements (0, 1, 2, etc).
   *   
   * @param $element
   *
   * @return 
   *   A Drupal Form array containing the settings form for this field.
   *
   * @ingroup tripal_map_includes
   */
  public function form(&$widget, &$form, &$form_state, $langcode, $items, $delta, $element) {
  
  	parent::form($widget, $form, $form_state, $langcode, $items, $delta, $element);
  	$widget['value'] = array(
  			'#type' => 'value',
  			'#value' => 'MapViewer marker map positions',
  	);
  }

  
  /**
   * function: validate
   *   Performs validation of the widgetForm to ensure that form values are entered correctly.
   *   
   * @param $element
   *   
   * @param &$form
   *   The form structure where widgets are being attached to. This might be a
   *   full form structure, or a sub-element of a larger form.
   *   
   * @param &$form_state
   *   An associative array containing the current state of the form.
   *   
   * @param $langcode
   *   The language associated with $items.
   *   
   * @param $delta
   *   The order of this item in the array of subelements (0, 1, 2, etc).
   *      
   * @return 
   *    None.
   *    
   * @ingroup tripal_map_includes
   */
  public function validate($element, $form, &$form_state, $langcode, $delta) {
  
  }

  
  /**
   * function: submit
   *  Performs extra commands when the entity form is submitted.
   *
   *  @param $form
   *    The submitted form array.
   *    
   *  @param $form_state.
   *    The form state array.
   *    
   *  @param $entity_type
   *    The type of $entity.
   *    
   *  @param $entity
   *    The entity for the operation.
   *
   * @param $langcode
   *   The language associated with $items.
   *
   * @param $delta
   *   The order of this item in the array of subelements (0, 1, 2, etc).
   *
   * @return
   *    None.
   *
   * @ingroup tripal_map_includes
   */
  public function submit($form, &$form_state, $entity_type, $entity, $langcode, $delta) {
  }

}
