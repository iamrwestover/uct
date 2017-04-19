<?php

/**
 * @file
 * UIkit's theme implementation to display a single Drupal page.
 *
 * The doctype, html, head and body tags are not in this template. Instead they
 * can be found in the html.tpl.php template in this directory.
 *
 * Available variables:
 *
 * General utility variables:
 * - $base_path: The base URL path of the Drupal installation. At the very
 *   least, this will always default to /.
 * - $directory: The directory the template is located in, e.g. modules/system
 *   or themes/bartik.
 * - $is_front: TRUE if the current page is the front page.
 * - $logged_in: TRUE if the user is registered and signed in.
 * - $is_admin: TRUE if the user has permission to access administration pages.
 *
 * Site identity:
 * - $front_page: The URL of the front page. Use this instead of $base_path,
 *   when linking to the front page. This includes the language domain or
 *   prefix.
 * - $logo: The path to the logo image, as defined in theme configuration.
 * - $site_name: The name of the site, empty when display has been disabled
 *   in theme settings.
 * - $site_slogan: The slogan of the site, empty when display has been disabled
 *   in theme settings.
 *
 * Navigation:
 * - $main_menu (array): An array containing the Main menu links for the
 *   site, if they have been configured.
 * - $secondary_menu (array): An array containing the Secondary menu links for
 *   the site, if they have been configured.
 * - $breadcrumb: The breadcrumb trail for the current page.
 *
 * Page content (in order of occurrence in the default page.tpl.php):
 * - $title_prefix (array): An array containing additional output populated by
 *   modules, intended to be displayed in front of the main title tag that
 *   appears in the template.
 * - $title: The page title, for use in the actual HTML content.
 * - $title_suffix (array): An array containing additional output populated by
 *   modules, intended to be displayed after the main title tag that appears in
 *   the template.
 * - $messages: HTML for status and error messages. Should be displayed
 *   prominently.
 * - $tabs (array): Tabs linking to any sub-pages beneath the current page
 *   (e.g., the view and edit tabs when displaying a node).
 * - $action_links (array): Actions local to the page, such as 'Add menu' on the
 *   menu administration interface.
 * - $feed_icons: A string of all feed icons for the current page.
 * - $node: The node object, if there is an automatically-loaded node
 *   associated with the page, and the node ID is the second argument
 *   in the page's path (e.g. node/12345 and node/12345/revisions, but not
 *   comment/reply/12345).
 *
 * Regions:
 * - $page['navigation']: Items for the UIkit navbar component.
 * - $page['header']: Items for the header region, including the site slogan.
 * - $page['highlighted']: Items for the highlighted content region.
 * - $page['help']: Dynamic help text, mostly for admin pages.
 * - $page['content']: The main content of the current page.
 * - $page['sidebar_first']: Items for the first sidebar.
 * - $page['sidebar_second']: Items for the second sidebar.
 * - $page['footer']: Items for the footer region.
 *
 * @see template_preprocess()
 * @see template_preprocess_page()
 * @see template_process()
 * @see uikit_preprocess_page()
 * @see uikit_process_page()
 * @see html.tpl.php
 *
 * @ingroup uikit_themeable
 */
?>
<!--<header--><?php //print $header_attributes; ?><!-->
<!--  <nav--><?php //print $navbar_attributes; ?><!-->
<!--    <a href="#menu-toggle" class="uk-navbar-toggle" data-uk-offcanvas="{mode:'reveal'}"></a>-->
<!--    --><?php //if ($logo): ?>
<!--      <a href="--><?php //print $front_page; ?><!--" id="logo-large" class="uk-navbar-brand uk-hidden-small" title="--><?php //print t('Home'); ?><!--" rel="home">-->
<!--        <img src="--><?php //print $logo; ?><!--" alt="--><?php //print t('Home'); ?><!--"/>-->
<!--      </a>-->
<!--    --><?php //endif; ?>
<!---->
<!--    --><?php //if ($site_name): ?>
<!--      <a href="--><?php //print $front_page; ?><!--" id="site-name" class="uk-navbar-brand" title="--><?php //print t('Home'); ?><!--" rel="home">-->
<!--        <span class="uk-hidden-small">--><?php //print $site_name; ?><!--</span>-->
<!--        <span class="uk-hidden-medium uk-hidden-large">UCT</span>-->
<!--      </a>-->
<!--    --><?php //endif; ?>
<!---->
<!--    --><?php //if ($main_menu || $secondary_menu || $navbar_menus): ?>
<!--      --><?php //print $navbar_main; ?>
<!--      --><?php //print $navbar_secondary; ?>
<!--      --><?php //print $navbar_menus; ?>
<!--    --><?php //endif; ?>
<!--  </nav>-->
<!--</header>-->

<div id="header-bar">
  <div class="uk-grid">
    <div class="uk-width-small-1-2">
      <?php if ($breadcrumb && $display_breadcrumb): ?>
      <div id="breadcrumbs">
        <?php print $breadcrumb; ?>
      </div>
      <?php endif; ?>
    </div>

    <div class="uk-width-small-1-2 uk-hidden-small">
      <div class="uk-text-right">
        <a href="#av-quick-search" class="uk-icon-search uk-icon-hover uk-margin-small-right" data-uk-modal data-ks="F3"></a>
        <a href="/av/help" class="uk-icon-small uk-icon-question-circle uk-icon-hover uk-margin-small-right" style="color: #659f13" title="Help"></a>
        <i class="uk-icon-calendar"></i>&nbsp;&nbsp;<?php print format_date(time(), 'custom', 'l, F j, Y'); ?>
      </div>
    </div>
  </div>
</div>
<?php
//$memory_usage = memory_get_peak_usage() / 1024 / 1024;
//$memory_limit = (int) ini_get('memory_limit');
//$memory_usage_percentage = $memory_usage / $memory_limit * 100;
//echo number_format($memory_usage, 2) . ' - ' . number_format($memory_limit, 2) . ' - ' . number_format($memory_usage_percentage, 2) . '%';
?>

<!-- This is the modal -->
<div id="av-quick-search" class="uk-modal">
  <div class="uk-modal-dialog uk-text-center" style="width: 500px;">
    <div class="uk-modal-header">
      <h2><i class="uk-icon-search uk-margin-right"></i>Quick Transaction Search</h2>
    </div>
    <div class="uk-form">
      <input id="av-quick-txn-id" type="text" placeholder="enter transaction id" class="uk-form-large uk-text-center">
      <div class="uk-text-muted uk-margin-top">ENTER to open on current page.<br />SHIFT+ENTER to open on a new window.</div>
    </div>
  </div>
</div>

<div id="page-outer-wrapper" class="uk-margin-left uk-margin-right">
  <div class="uk-grid uk-grid-collapse uk-hidden printable">
    <div class="uk-width-3-5">
      <div id="print-header">
        <div class="site-name uk-text-bold"><?php print strtoupper($site_name); ?></div>
        <div class="site-address"><?php print check_plain(variable_get('av_company_address', NULL)); ?></div>
        <div class="site-contact-number"><?php print check_plain(variable_get('av_company_contact_number', NULL)); ?></div>
      </div>
    </div>
    <div class="uk-width-2-5 uk-text-right">
      <?php if ($title): ?><div class="uk-article-title"><?php print $title; ?></div><?php endif; ?>
    </div>
    <hr class="uk-width-1-1 uk-margin-small uk-margin-top" />
  </div>

  <div<?php print $page_container_attributes; ?>>
    <?php if ($site_slogan): ?>
      <div id="site-slogan">
        <div class="uk-margin-bottom"><?php print $site_slogan; ?></div>
      </div>
    <?php endif; ?>

    <div class="uk-grid" data-uk-grid-margin>
      <?php if ($page['header']): ?>
        <?php print render($page['header']); ?>
      <?php endif; ?>

      <?php if ($page['highlighted']): ?>
        <div id="highlighted" class="uk-width-1-1">
          <?php print render($page['highlighted']); ?>
        </div>
      <?php endif; ?>

      <div<?php print $content_attributes; ?>>
        <?php print render($title_prefix); ?>
        <?php if ($title): ?>
          <div id="page-title" class="not-printable uk-article-title uk-margin-bottom uk-margin-top"><?php print $title; ?></div><?php endif; ?>
        <?php print render($title_suffix); ?>

        <?php if ($page['intro']): ?>
          <?php print render($page['intro']); ?>
        <?php endif; ?>

        <?php if ($tabs): ?>
          <?php print render($tabs); ?>
        <?php endif; ?>

        <?php if ($messages): ?>
          <div id="messages" class="not-printable uk-width-1-1 uk-panel-">
            <?php print $messages; ?>
          </div>
        <?php endif; ?>

        <?php print render($page['help']); ?>

        <?php if ($action_links): ?>
          <ul class="action-links uk-subnav uk-subnav-pill"><?php print render($action_links); ?></ul>
        <?php endif; ?>

        <?php print render($page['content']); ?>
        <?php print $feed_icons; ?>
      </div>

      <?php if ($page['sidebar_first']): ?>

        <div<?php print $sidebar_first_attributes; ?>>
          <?php print render($page['sidebar_first']); ?>
        </div>
      <?php endif; ?>

      <?php if ($page['sidebar_second']): ?>
        <div<?php print $sidebar_second_attributes; ?>>
          <?php print render($page['sidebar_second']); ?>
        </div>
      <?php endif; ?>

    </div>

    <?php if ($page['footer']): ?>
      <div class="uk-grid" data-uk-grid-margin>
          <div id="footer" class="uk-width-1-1">
            <?php print render($page['footer']); ?>
          </div>
      </div>
    <?php endif; ?>

  </div>
</div>

<?php if ($offcanvas_main || $offcanvas_secondary): ?>
  <div id="menu-toggle" class="uk-offcanvas uk-active">
    <div class="uk-offcanvas-bar">
      <?php print $offcanvas_main; ?>
      <?php print $offcanvas_secondary; ?>
    </div>
  </div>
<?php endif; ?>
