<?php

/**
 * @file
 * Administration pages provided by Webform module.
 */

/**
 * Menu callback for admin/config/content/webform.
 */
function zendesk_webform_admin_settings() {

  $form['zendesk_webform_api_enabled'] = array(
    '#title' => t('Enable Zendesk for Webforms'),
    '#type' => 'radios',
    '#default_value' => variable_get('zendesk_webform_api_enabled', 0),
    '#options' => array(t('No'),t('Yes')),
  );
  $form['zendesk_api'] = array(
    '#type' => 'fieldset',
    '#title' => t('Zendesk credentials'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#states' => array(
      'visible' => array(
        ':input[name="zendesk_webform_api_enabled"]' => array('value' => 1),
      ),
    ),
  );

  $form['zendesk_api']['zendesk_webform_api_url']  = array(
    '#type' => 'textfield',
    '#title' => t('Zendesk API url'),
    '#default_value' => variable_get('zendesk_webform_api_url', ''),
    '#description' => t('which zendesk instance shall form submission be pushed to. See http://xxx.zendesk.com/agent/#/admin/api'),
  );

  $form['zendesk_api']['zendesk_webform_api_user']  = array(
    '#type' => 'textfield',
    '#title' => t('Zendesk admin users email address'),
    '#default_value' => variable_get('zendesk_webform_api_user', ''),
    '#description' => t('Which user shall requests be submitted'),
  );

  $form['zendesk_api']['zendesk_webform_api_key']  = array(
    '#type' => 'textfield',
    '#title' => t('Zendesk API token'),
    '#default_value' => variable_get('zendesk_webform_api_key', ''),
    '#description' => t('Zendesk instance API token. This is unique to each instance. See http://xxx.zendesk.com/agent/#/admin/api'),
  );

  //if zendesk has been enabled, we can now test the credentials
  //if the credentials are correct, show the parent groups. so that we can
  //associate the webforms with given groups subgroups later on.
  if (variable_get('zendesk_webform_api_enabled', 0)){
    $response = zendesk_webform_api_call('', 'users/me.json');

    if (is_object($response) & !isset($response->error)) {
      drupal_set_message(t('Zendesk credentials configured and working correctly.'));
      //if all is configured and working, show the parent groups
      $default_group  = variable_get('zendesk_webform_group');
      $zendesk_groups = zendesk_webform_list_group_field($default_group, '');
      array_push($form, $zendesk_groups);
    }
    else {
      $error = (is_object($response)) ? $response->error->title:"Cant connect";
      drupal_set_message(t('Zendesk returned the following error: "').$error.t('". Please review the credentials provided.'), 'error');
    }
  }

  $form = system_settings_form($form);
  return $form;
}
