<?php
$view_mode = !empty($form['#av_view_mode']) ;
$vertical_margin_class = $view_mode ? 'uk-margin-bottom' : 'uk-margin-small-bottom';

// Remove fieldset titles.
$form['info']['#access'] = FALSE;
$form['contact']['#access'] = FALSE;
$form['address']['#access'] = FALSE;
$form['payment']['#access'] = FALSE;
$form['misc']['#access'] = FALSE;

//unset($form['contact']['email']['#title']);
//unset($form['contact']['contact_number']['#title']);
//unset($form['contact']['website']['#title']);
//$form['contact']['email']['#attributes']['placeholder'] = 'e-mail address';
//$form['contact']['contact_number']['#attributes']['placeholder'] = 'contact number';
//$form['contact']['website']['#attributes']['placeholder'] = 'website';


// Remove resize handle from textareas.
//$form['misc']['notes']['#resizable'] = FALSE;
$form['address']['address']['#resizable'] = FALSE;
$form['address']['address']['#attributes']['rows'] = 3;

// Simplify address fields.
//unset($form['address']['city']['#title']);
//unset($form['address']['province']['#title']);
//unset($form['address']['zip_code']['#title']);
$form['address']['address']['#attributes']['placeholder'] = 'Building / Unit # / Street / etc.';
//$form['address']['city']['#attributes']['placeholder'] = 'City';
//$form['address']['province']['#attributes']['placeholder'] = 'Province';
//$form['address']['zip_code']['#attributes']['placeholder'] = 'ZIP Code';

$view_mode = !empty($form['#av_view_mode']);

$transactions_html = drupal_render($form['transactions']);
$payment_method_html = drupal_render($form['payment']['payment_method_id']);
$area_html = drupal_render($form['info']['area_id']);
$website_html = drupal_render($form['contact']['website']);
?>

<?php if ($view_mode): ?>
  <h1 class="uk-text-center uk-margin-small-top"><?php print drupal_render($form['info']['display_name']); ?></h1>
<?php endif; ?>

<div class="uk-grid uk-grid-large">
  <div class="uk-width-1-2">
    <div class="uk-grid uk-grid-small">
      <div class="uk-width-1-2 <?php print $vertical_margin_class; ?>"><?php print drupal_render($form['info']['display_name']); ?></div>
      <div class="uk-width-1-2 <?php print $vertical_margin_class; ?>"><?php print drupal_render($form['info']['agent_name']); ?></div>
      <!--<div class="uk-width-1-3 --><?php //print $vertical_margin_class; ?><!--">--><?php //print drupal_render($form['info']['code']); ?><!--</div>-->

      <div class="uk-width-2-3 <?php print $vertical_margin_class; ?>"><?php print drupal_render($form['info']['company']); ?></div>
      <div class="uk-width-1-3 <?php print $vertical_margin_class; ?>"><?php print drupal_render($form['info']['category_id']); ?></div>

<!--      <div class="uk-width-1-10 <?php print $vertical_margin_class; ?>">--><?php //print drupal_render($form['info']['title']); ?><!--</div>-->
      <div class="uk-width-2-6 <?php print $vertical_margin_class; ?>"><?php print drupal_render($form['info']['first_name']); ?></div>
      <div class="uk-width-2-6 <?php print $vertical_margin_class; ?>"><?php print drupal_render($form['info']['middle_name']); ?></div>
      <div class="uk-width-2-6 <?php print $vertical_margin_class; ?>"><?php print drupal_render($form['info']['last_name']); ?></div>


      <div class="uk-width-1-3 <?php print $vertical_margin_class; ?>"><?php print drupal_render($form['info']['_created']); ?></div>
    </div>
  </div>

  <div class="uk-width-1-2">
    <div class="uk-grid uk-grid-small">
      <?php if ($area_html): ?>
        <div class="uk-width-1-1 <?php print $vertical_margin_class; ?>"><?php print $area_html; ?></div>
      <?php endif; ?>
      <div class="uk-width-1-2 <?php print $vertical_margin_class; ?>"><?php print drupal_render($form['contact']['email']); ?></div>
      <div class="uk-width-1-2 <?php print $vertical_margin_class; ?>"><?php print drupal_render($form['contact']['contact_number']); ?></div>
      <?php if ($website_html): ?>
        <div class="uk-width-1-2 <?php print $vertical_margin_class; ?>"><?php print $website_html; ?></div>
      <?php endif; ?>
      <!--<div class="uk-width-1-3 --><?php //print $vertical_margin_class; ?><!--">--><?php //print drupal_render($form['contact']['phone2']); ?><!--</div>-->
      <!--<div class="uk-width-1-3 --><?php //print $vertical_margin_class; ?><!--">--><?php //print drupal_render($form['contact']['phone3']); ?><!--</div>-->

      <!--<div class="uk-width-1-1 --><?php //print $vertical_margin_class; ?><!--">--><?php //print drupal_render($form['contact']['website']); ?><!--</div>-->


    </div>
  </div>

  <div class="uk-width-1-1 uk-margin-top">
    <ul class="uk-tab" data-uk-tab="{connect:'#more-info'}">
      <?php if ($transactions_html): ?>
        <li><a href="">Recent Transactions</a></li>
      <?php endif; ?>
      <li><a href="">Address</a></li>
      <li><a href="">Payment</a></li>
    </ul>
    <ul id="more-info" class="uk-switcher">
      <?php if ($transactions_html): ?>
        <li class="uk-panel uk-panel-box">
          <div class="">
            <?php print $transactions_html; ?>
          </div>
        </li>
      <?php endif; ?>

      <li class="uk-panel uk-panel-box">

        <div class="uk-grid">
          <div class="uk-width-1-2">
            <div class="uk-grid uk-grid-small">
              <div class="uk-width-1-1 <?php print $vertical_margin_class; ?>"><?php print drupal_render($form['address']['address']); ?></div>
              <div class="uk-width-2-6 <?php print $vertical_margin_class; ?>"><?php print drupal_render($form['address']['city']); ?></div>
              <div class="uk-width-2-6 <?php print $vertical_margin_class; ?>"><?php print drupal_render($form['address']['province']); ?></div>
              <div class="uk-width-2-6 <?php print $vertical_margin_class; ?>"><?php print drupal_render($form['address']['zip_code']); ?></div>
            </div>
          </div>

          <div class="uk-width-1-2">
            <?php print drupal_render($form['address']['same_addr']); ?>
          </div>
        </div>

      </li>
      <li class="uk-panel uk-panel-box">
        <div class="uk-grid">
          <div class="uk-width-1-2">
            <div class="uk-grid uk-grid-small">
              <?php if ($payment_method_html): ?>
                <div class="uk-width-1-1 <?php print $vertical_margin_class; ?>"><?php print $payment_method_html; ?></div>
              <?php endif; ?>
              <div class="uk-width-1-3 <?php print $vertical_margin_class; ?>"><?php print drupal_render($form['info']['term_id']); ?></div>
              <div class="uk-width-1-3 <?php print $vertical_margin_class; ?>"><?php print drupal_render($form['info']['discount_type']); ?></div>
              <div class="uk-width-1-3 <?php print $vertical_margin_class; ?>"><?php print drupal_render($form['info']['discount_value']); ?></div>
            </div>
          </div>

          <div class="uk-width-1-2">
            <div class="uk-grid uk-grid-small">
              <div class="uk-width-1-2 <?php print $vertical_margin_class; ?>"><?php print drupal_render($form['payment']['opening_balance']); ?></div>
              <div class="uk-width-1-2 <?php print $vertical_margin_class; ?>"><?php print drupal_render($form['payment']['credit_limit']); ?></div>
            </div>
          </div>
        </div>
      </li>
    </ul>
  </div>
</div>

<?php
//$close_btn_label = empty($form['id']['#value']) ? 'Cancel' : 'Close';
//$form['buttons']['submit']['#attributes']['class'][] = 'uk-button-primary';
//$form['buttons']['cancel']['#markup'] = l($close_btn_label, 'av/customers', array('attributes' => array('class' => array('uk-button'))));
print drupal_render_children($form);