<?php
  $modal_options = empty($form['#modal_options']) ? array() : $form['#modal_options'];
  extract($modal_options);

  $close_btn_label = isset($close_btn_label) ? $close_btn_label : t('Cancel');
  $onclick_script = isset($onclick_script) ? $onclick_script : '';
  $modal_classes = isset($modal_classes) ? (' ' . implode(' ', $modal_classes)) : '';

  $buttons_html = drupal_render($form['buttons']);
?>
<!-- Do not remove this comment line. -->
<div id="uk-modal-<?php print $modal_id; ?>" class="uk-modal">
  <div class="uk-modal-dialog<?php print $modal_classes; ?>">
    <?php if (!empty($show_close_button)): ?>
      <a class="uk-modal-close uk-close"></a>
    <?php endif; ?>
    <?php if (!empty($modal_title)): ?>
      <div class="uk-modal-header"><h2><?php print $modal_title; ?></h2></div>
    <?php endif; ?>
    <div class="uk-modal-body"><?php print drupal_render_children($form); ?></div>
    <div class="uk-modal-footer uk-text-right">
      <?php print $buttons_html; ?>
    </div>
  </div>
</div>