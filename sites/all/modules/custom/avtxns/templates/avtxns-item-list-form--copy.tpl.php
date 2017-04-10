<?php
$view_mode = !empty($form['#av_view_mode']);
hide($form['header_total']);
hide($form['footer_total']);
//$form['prod_add_btn']['#attributes']['class'][] = 'av-ajax-trigger';
//$form['transaction_date']['#attributes']['data-uk-datepicker'] = "{format:'MMM. DD, YYYY'}";
//$form['buttons']['submit']['#attributes']['class'][] = 'uk-button-primary';
//$form['buttons']['submit_and_send']['#attributes']['class'][] = 'uk-button-primary';
//$form['buttons']['submit_and_send']['#attributes']['disabled'] = TRUE;
//$form['discount_value']['#attributes']['class'][] = 'uk-text-right';

//$form['message']['#attributes']['rows'] = 2;
//$form['memo']['#attributes']['rows'] = 2;
//$form['client_id']['#attributes']['placeholder'] = 'Enter vendor name or company name';


$element_bottom_margin = $view_mode ? '' : ' uk-margin-small-bottom';




//$table_rows = array(
//  array(
//    'data' => array('x' => drupal_render($form['form_body'])),
//  ),
//);
//$table = array(
//  '#theme' => 'table',
//  '#header' => array('s' => drupal_render($form['form_header'])),
//  '#rows' => $table_rows,
//  //'#empty' => '<div class="uk-text-muted">' . t('-') . '</div>',
//  '#attributes' => array(
//    //'id' => 'transaction-list-table',
//    'class' => array('uk-table-condensed printable'),
//  ),
//);

//print drupal_render($table);

print '<div class="printable">';
print drupal_render($form['form_header']);
print drupal_render($form['form_body']);
print '</div>';


//$close_btn_label = empty($form['id']['#value']) ? 'Cancel' : 'Close';
////$form['buttons']['submit']['#attributes']['class'][] = 'uk-button-primary';
//$form['buttons']['cancel']['#markup'] = l($close_btn_label, 'av/expenses', array('attributes' => array('class' => array('uk-button'))));
print drupal_render_children($form);