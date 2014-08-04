zendesk_webform
===============

Use drupal webforms to submit zendesk tickets. Build relationship between forms and groups. Webform fields and ticketfields


TODO
-Write cron job to clean up the user_fields/tickets_fields/groups cache.
-Admin settings: default_values for the fields are currently not set
-Admin settings: what should be enabled by default (look at core ticket/user fields)
-Admin settings: theme Zendesk Fields checkboxes into a table
-Webform: define table for building relationship between webform fields & zendesk fields
-Webform: hook_submit for webforms, currently tickets are not submitted to zendesk.
