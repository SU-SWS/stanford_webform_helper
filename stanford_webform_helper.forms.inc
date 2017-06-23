<?php

/**
 * @file
 * stanford_webform_helper.forms.inc
 */

/**
 * SFTP Configuration form.
 */
function stanford_webform_helper_sftp_form($form, &$form_state, $node) {
  $dir = 'public://webform';
  if (!file_prepare_directory($dir, FILE_CREATE_DIRECTORY)) {
    drupal_set_message(t('Unable to create directory !dir', array('!dir' => $dir)), 'error');
    return;
  }

  $form_state['node'] = $node;
  $sftp = variable_get('stanford_webform_helper_sftp', array());
dpm($sftp);
  $webform_settings = array(
    'ftp' => 0,
    'enabled' => 0,
    'username' => '',
    'password' => '',
    'sftp' => '',
    'directory' => '',
    'filename' => 'webform_[submission:sid]_[submission:date:raw]',
    'port' => '22',
    'field_order' => array(),
    'customize_field_order' => 0,
  );

  if (isset($sftp[$node->nid])) {
    $webform_settings = $sftp[$node->nid];
  }

  $form = array();
  $form['#tree'] = TRUE;
  $form['enabled'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable SFTP Upload'),
    '#default_value' => $webform_settings['enabled'],
  );
  $form['sftp_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('SFTP Settings'),
    '#states' => array(
      'visible' => array(
        ':input[name="enabled"]' => array('checked' => TRUE),
      ),
    ),
  );

  $form['sftp_settings']['ftp'] = array(
    '#type' => 'checkbox',
    '#title' => t('Use Standard FTP'),
    '#description' => t('If SFTP is not supported, try FTP protocols'),
    '#default_value' => $webform_settings['ftp'],
  );
  $form['sftp_settings']['username'] = array(
    '#type' => 'textfield',
    '#title' => t('Username'),
    '#default_value' => $webform_settings['username'],
  );
  $form['sftp_settings']['password'] = array(
    '#type' => 'password',
    '#title' => t('Password'),
    '#default_value' => '',
  );
  if ($webform_settings['password']) {
    $form['sftp_settings']['password']['#attributes']['placeholder'] = t('Already Entered');
  }
  $form['sftp_settings']['sftp'] = array(
    '#type' => 'textfield',
    '#title' => t('SFTP Host'),
    '#default_value' => $webform_settings['sftp'],
  );
  $form['sftp_settings']['port'] = array(
    '#type' => 'textfield',
    '#title' => t('Port'),
    '#default_value' => $webform_settings['port'],
  );
  $form['sftp_settings']['directory'] = array(
    '#type' => 'textfield',
    '#title' => t('Remote Directory'),
    '#description' => t('Path on the SFTP server to upload the file. No trailing slash.'),
    '#default_value' => $webform_settings['directory'] ? $webform_settings['directory'] : '/',
  );
  $form['sftp_settings']['filename'] = array(
    '#type' => 'textfield',
    '#title' => t('filename'),
    '#description' => t('Name of the file to create on the SFTP server excluding the file extension.'),
    '#default_value' => $webform_settings['filename'],
  );

  $form['sftp_settings']['customize_field_order'] = array(
    '#type' => 'checkbox',
    '#title' => t('Customize csv field order'),
    '#default_value' => $webform_settings['customize_field_order'],
  );
  $form['sftp_settings']['field_order'] = array(
    '#type' => 'fieldset',
    '#title' => t('Field Order'),
    '#states' => array(
      'visible' => array(
        'input[name*="customize_field_order"]' => array('checked' => TRUE),
      ),
    ),
  );

  $field_order = [];
  foreach ($node->webform['components'] as $cid => $component) {
    $field_order[$cid]['key'] = array(
      '#markup' => $component['form_key'],
    );
    $field_order[$cid]['weight'] = array(
      '#type' => 'weight',
      '#title' => t('Weight'),
      '#title_display' => 'invisible',
      '#default_value' => isset($webform_settings['field_order'][$cid]) ? $webform_settings['field_order'][$cid] : $component['weight'],
    );
  }

  uasort($field_order, function ($array1, $array2) {
    if ($array1['weight']['#default_value'] > $array2['weight']['#default_value']) {
      return TRUE;
    }
    return FALSE;
  });

  $form['sftp_settings']['field_order'] += $field_order;
  $form['sftp_settings']['token_help'] = array(
    '#theme' => 'token_tree',
    '#token_types' => array('submission'),
  );
  $form['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  return $form;
}

/**
 * SFTP configuration form validation.
 */
function stanford_webform_helper_sftp_form_validate($form, &$form_state) {
  $values = &$form_state['values']['sftp_settings'];
  if ($form_state['values']['enabled']) {
    $original_settings = variable_get('stanford_webform_helper_sftp', array());
    if (isset($original_settings[$form_state['node']->nid])) {
      $original_settings = $original_settings[$form_state['node']->nid];
    }
    $required_fields = array(
      'username' => t('Username'),
      'sftp' => t('SFTP Host'),
      'port' => t('Port'),
      'filename' => t('File Name'),
    );

    foreach ($required_fields as $name => $label) {
      if (!$values[$name]) {
        form_set_error($name, t('!label field is required', array('!label' => $label)));
      }
    }

    if (!isset($original_settings['password']) && !$values['password']) {
      form_set_error('password', t('Password field is required'));
    }
  }

  $values['directory'] = rtrim($values['directory'], '/ ');
  $values['directory'] = ltrim($values['directory'], ' ');
}

/**
 * SFTP Configuration form Submit.
 */
function stanford_webform_helper_sftp_form_submit($form, &$form_state) {
  $form_values = $form_state['values']['sftp_settings'];
  $sftp = variable_get('stanford_webform_helper_sftp', array());
  $old_password = '';
  if (isset($sftp[$form_state['node']->nid])) {
    $old_password = $sftp[$form_state['node']->nid]['password'];
  }
  $webform_settings = array(
    'ftp' => $form_values['ftp'],
    'enabled' => $form_state['values']['enabled'],
    'username' => $form_values['username'],
    'password' => $old_password,
    'sftp' => $form_values['sftp'],
    'port' => $form_values['port'],
    'directory' => $form_values['directory'],
    'filename' => $form_values['filename'],
    'customize_field_order' => $form_values['customize_field_order'],
  );
  if ($form_values['password']) {
    $webform_settings['password'] = unserialize(encrypt($form_state['values']['password']));
  }

  foreach ($form_values['field_order'] as &$weight) {
    $weight = $weight['weight'];
  }
  asort($form_values['field_order']);

  $webform_settings['field_order'] = $form_values['field_order'];
  $sftp[$form_state['node']->nid] = $webform_settings;
  variable_set('stanford_webform_helper_sftp', $sftp);
  drupal_set_message(t('Settings Saved'));
}
