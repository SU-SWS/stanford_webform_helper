<?php

/**
 * @file
 * stanford_webform_helper.forms.inc
 */

/**
 * SFTP Configuration form.
 */
function stanford_webform_helper_sftp_form($form, &$form_state, $node) {
  if (!function_exists('ssh2_connect')) {
    drupal_set_message(t('SSH2 plugin is not available'), 'error');
    return $form;
  }

  $form_state['node'] = $node;
  $sftp = variable_get('stanford_webform_helper_sftp', array());
  $webform_settings = array(
    'enabled' => 0,
    'username' => '',
    'password' => '',
    'sftp' => '',
    'directory' => '',
    'port' => '22',
  );
  if (isset($sftp[$node->nid])) {
    $webform_settings = $sftp[$node->nid];
  }

  $form = array();
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
  $form['sftp_settings']['directory'] = array(
    '#type' => 'textfield',
    '#title' => t('Remote Directory'),
    '#description' => t('Path on the SFTP server to upload the file. No trailing slash.'),
    '#default_value' => $webform_settings['directory'],
  );
  $form['sftp_settings']['port'] = array(
    '#type' => 'textfield',
    '#title' => t('Port'),
    '#default_value' => $webform_settings['port'],
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
  $values = $form_state['values'];
  if ($values['enabled']) {
    $original_settings = variable_get('stanford_webform_helper_sftp', array());
    if (!$values['username']) {
      form_set_error('username', t('Username field is required'));
    }
    if (!$values['password'] && !isset($original_settings['password'])) {
      form_set_error('password', t('Password field is required'));
    }
    if (!$values['sftp']) {
      form_set_error('sftp', t('SFTP Host field is required'));
    }
    if (!$values['directory']) {
      form_set_error('directory', t('Remote Directory field is required'));
    }
    if (!$values['port']) {
      form_set_error('port', t('Port field is required'));
    }
  }

  $form_state['values']['directory'] = rtrim($form_state['values']['directory'], '/ ');
  $form_state['values']['directory'] = rtrim($form_state['values']['directory'], ' ');
}

/**
 * SFTP Configuration form Submit.
 */
function stanford_webform_helper_sftp_form_submit($form, &$form_state) {
  $sftp = variable_get('stanford_webform_helper_sftp', array());
  $old_password = '';
  if (isset($sftp[$form_state['node']->nid])) {
    $old_password = $sftp[$form_state['node']->nid]['password'];
  }
  $webform_settings = array(
    'enabled' => $form_state['values']['enabled'],
    'username' => $form_state['values']['username'],
    'password' => $old_password,
    'sftp' => $form_state['values']['sftp'],
    'port' => $form_state['values']['port'],
    'directory' => $form_state['values']['directory'],
  );
  if ($form_state['values']['password']) {
    $webform_settings['password'] = unserialize(encrypt($form_state['values']['password']));
  }

  $sftp[$form_state['node']->nid] = $webform_settings;
  variable_set('stanford_webform_helper_sftp', $sftp);
}
