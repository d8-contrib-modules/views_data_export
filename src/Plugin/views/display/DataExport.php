<?php

namespace Drupal\views_data_export\Plugin\views\display;

use Drupal\Core\Form\FormStateInterface;
use Drupal\rest\Plugin\views\display\RestExport;

/**
 * Provides a data export display plugin.
 *
 * This overrides the REST Export display to make labeling clearer on the admin
 * UI, and to allow attaching of these to other displays.
 *
 * @ingroup views_display_plugins
 *
 * @ViewsDisplay(
 *   id = "data_export",
 *   title = @Translation("Data export"),
 *   help = @Translation("Export the view results to a file. Can handle very large result sets."),
 *   uses_route = TRUE,
 *   admin = @Translation("Data export"),
 *   returns_response = TRUE
 * )
 */
class DataExport extends RestExport {

  /**
   * {@inheritdoc}
   */
  protected function defineOptions() {
    $options = parent::defineOptions();

    // Set the default style plugin, and default to fields.
    $options['style']['contains']['type']['default'] = 'data_export';
    $options['row']['contains']['type']['default'] = 'data_field';

    return $options;
  }

  /**
   * {@inheritdoc}
   */
  public function buildOptionsForm(&$form, FormStateInterface $form_state) {
    parent::buildOptionsForm($form, $form_state);

    // Remove the 'serializer' option to avoid confusion.
    switch ($form_state->get('section')) {
      case 'style':
        unset($form['style']['type']['#options']['serializer']);
    }
  }

}
