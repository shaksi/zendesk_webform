<?php

/**
 * @file
 * Administration pages provided by Webform module.
 */

function zendesk_webform_admin_languages() {
  $items = array();
  $enabled_langs = FALSE;
  $languages = language_list('enabled');
  if (isset($languages[1]) && count($languages[1])) {
    $enabled_langs = $languages[1];
  }

  $header = array(t('Name'), array('data' => t('Operations'), /*'colspan' => '2'*/));
  $rows = array();

  if ($enabled_langs) {
    foreach ($enabled_langs as $key => $language) {
      $row = array($language->name . ' <small>(' . t('Machine name') . ': ' . $language->language . ')</small>');
      // Set the edit column.
      $row[] = array('data' => l(t('edit'), 'admin/config/content/webform/zendesk/languages/' . $language->language . ''));

      $rows[] = $row;
    }
  }

  $build['zendesk_webform_admin_languages_table'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('No enabled languages available. <a href="@link">Add language</a>.', array('@link' => url('admin/config/regional/language/add'))),
  );
  return $build;
}

function zendesk_webform_language_admin_config_form($form, &$form_state, $lang) {
  $defaults = variable_get('zendesk_api', array());

  $form = array('#tree' => TRUE);
  $form['zendesk_api'][$lang]['status'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enabled'),
    '#default_value' => (isset($defaults[$lang]['status'])) ? $defaults[$lang]['status'] : 0,
    '#description' => t('Check this box to enable the zendesk functionallities for webforms in this specific language.'),
  );

  //if zendesk has been enabled, we can now test the credentials
  //if the credentials are correct, show the parent groups. so that we can
  //associate the webforms with given groups subgroups later on.
  if (isset($defaults[$lang]['status']) && $defaults[$lang]['status']){
    $response = zendesk_webform_api_call('', 'users/me.json');

    if (is_object($response) & !isset($response->error)) {
      drupal_set_message(t('Zendesk credentials configured and working correctly.'));

      //while the going is good, lets cache user/ticket fields
      zendesk_webform_cache_fields();
    }
    else {
      $error = (is_object($response)) ? $response->error->title:"Cant connect";
      drupal_set_message(t('Zendesk returned the following error: "').$error.'"', 'error');
    }

        //if all is configured and working, show the parent groups
    //We should already have cached ticket/user fields & groups from
    //zendesk_webform_cache_fields()
    // if ( variable_get('zendesk_webform_country_groups', FALSE)) {
    //   $default_group  = variable_get('zendesk_webform_group', FALSE);
    //   $zendesk_groups = zendesk_webform_list_group_field($default_group, '');
    //   $zendesk_groups['zendesk']['#states'] = $show_only_if_api_true;
    //   $zendesk_groups['zendesk']['zendesk_webform_group']=
    //   array_push($form, $zendesk_groups);
    // }

    //list out all the user/tickets so that we can preselect the ones that will
    //be available to the user in the webform component area.
    $fields = _zendesk_webform_fields_cleanup();

    if (count($fields) > 0) {
      $form['zendesk_api'][$lang]['fields'] = array(
        '#type' => 'fieldset',
        '#title' => t('Zendesk Fields'),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
        '#states' => array(
          'visible' => array(
            ':input[name="zendesk_api[' . $lang . '][status]"]' => array('checked' => TRUE),
          ),
        ),
      );

      $form['zendesk_api'][$lang]['fields']['zendesk_webform_allowed_fields_user']  = array(
        '#type' => 'checkboxes',
        '#title' => t('User fields'),
        '#options' => (array)$fields->user,
        '#default_value' => (isset($defaults[$lang]['fields']['zendesk_webform_allowed_fields_user'])) ? $defaults[$lang]['fields']['zendesk_webform_allowed_fields_user'] : array(),
        '#description' => t('This is list of all the fields available for zendesk user'),
      );

      $form['zendesk_api'][$lang]['fields']['zendesk_webform_allowed_fields_ticket']  = array(
        '#type' => 'checkboxes',
        '#title' => t('Ticket fields'),
        '#options' => (array)$fields->ticket,
        '#default_value' => (isset($defaults[$lang]['fields']['zendesk_webform_allowed_fields_ticket'])) ? $defaults[$lang]['fields']['zendesk_webform_allowed_fields_ticket'] : array(),
        '#description' => t('This is list of all the fields available for zendesk ticket'),
      );
    }
  }

  $form['zendesk_api']['language'] = array(
    '#type' => 'value',
    '#value' => $lang,
  );

  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );

  return $form;
}

function zendesk_webform_language_admin_config_form_submit($form, &$form_state) {
  $defaults = variable_get('zendesk_api', array());
  $values = $form_state['values'];
  if (isset($values['zendesk_api'])) {
    $defaults[$values['zendesk_api']['language']] = $values['zendesk_api'][$values['zendesk_api']['language']];
    variable_set('zendesk_api', $defaults);
  }
}

/**
 * Menu callback for admin/config/content/webform.
 */
function zendesk_webform_admin_settings() {
  $form['zendesk_api'] = array(
    '#type' => 'fieldset',
    '#title' => t('Zendesk credentials'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
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

  //settings form will do all the saving for us so need for _submit
  $form = system_settings_form($form);
  return $form;
}

/**
 * Cache userfields and tickets to save us
 * from hitting zendesk api unnecessarily (should go into cron.)
 */
function zendesk_webform_cache_fields($clear=FALSE) {
  //get list of current user_fields and cache them
  if (!variable_get('zendesk_webform_user_fields', FALSE) || $clear) {
    $user_fields = zendesk_webform_api_call('', 'user_fields.json','GET');
    variable_set('zendesk_webform_user_fields', serialize($user_fields));
  }

  //get a list of current ticket fields and cache them
  if (!variable_get('zendesk_webform_ticket_fields', FALSE) || $clear) {
    $ticket_fields = zendesk_webform_api_call('', 'ticket_fields.json','GET');
    variable_set('zendesk_webform_ticket_fields', serialize($ticket_fields));
  }

  //get a list of current country groups and cache them.
  if (!variable_get('zendesk_webform_country_groups', FALSE) || $clear) {
    $country_groups = zendesk_webform_api_call('', 'groups.json','GET');
    variable_set('zendesk_webform_country_groups', serialize($country_groups));
  }

}
