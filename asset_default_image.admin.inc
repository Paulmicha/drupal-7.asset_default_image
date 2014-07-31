<?php

/**
 * @file
 * Asset default image - admin pages
 */

/**
 * Settings form declaration
 */
function asset_default_image_settings_form() {
  $form = array();
  
  $form['asset_default_image_aid'] = array(
    '#title' => t("Default image asset ID"),
    '#description' => t("You may enter several image IDs by separating them with a comma : <code>,</code>"),
    '#type' => 'textfield',
    '#default_value' => variable_get('asset_default_image_aid', 0),
  );
  
  return system_settings_form($form);
}
