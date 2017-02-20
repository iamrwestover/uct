<div class="suk-panel suk-panel-box">
<span>Discounted total amount when paid</span>
<div class="uk-grid uk-grid-small">
  <?php foreach (element_children($form) as $key): ?>
    <div class="uk-width-1-5">
      <?php print drupal_render($form[$key]); ?>
    </div>
  <?php endforeach; ?>
</div>
</div>