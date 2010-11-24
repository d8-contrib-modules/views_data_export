<?php
// $Id$
/**
 * @file
 * Plugin include file for export style plugin.
 */

/**
 * Generalized style plugin for export plugins.
 *
 * @ingroup views_style_plugins
 */
class views_data_export_plugin_style_export_txt extends views_data_export_plugin_style_export {
  var $feed_text = 'Plain Text Document';
  var $feed_file = 'view-%view.txt';

  /**
   * Initialize plugin.
   *
   * Set feed image for shared rendering later.
   */
  function init(&$view, &$display, $options = NULL) {
    parent::init($view, $display, $options = NULL);
    $this->feed_image = drupal_get_path('module', 'views_data_export') . '/images/txt.png';
  }
}
