<hr />
<div class="uk-grid uk-grid-small uk-text-center" data-uk-grid-margin>
  <?php foreach (element_children($form) as $balance_key): ?>
    <div class="uk-width-1-6">
      <?php print drupal_render($form[$balance_key]); ?>
    </div>
  <?php endforeach; ?>
</div>
<hr />