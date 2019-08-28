<h2 id="backup-create"><?php print $help_title; ?></h2>
<p>It is highly recommended that no one else is using the system during backup creation.</p>
<ol>
  <li>Go to <?php print l('Backup page', 'admin/config/system/backup_migrate', array('attributes' => array('target' => '_blank'))); ?>.</li>
  <li>Under <span class="uk-text-bold">Backup my</span>, select "Default database".</li>
  <li>Under <span class="uk-text-bold">to</span>, select your preferred location where the backup will be saved.</li>
  <li>Under <span class="uk-text-bold">using</span>, select the type of backup you wish to save</li>
  <ul>
    <li><span class="uk-text-italic">Everything in the database</span> - this will create a backup of all data in the entire database. Highly recommended.</li>
    <li>
      <span class="uk-text-italic">Everything in the database EXCEPT Transactions</span> - transactions will be <span class="uk-text-bold uk-text-warning">excluded</span> from the backup. Only use this if you are planning to reset the system with a new database.</li>
    <li>
      <span class="uk-text-italic">Clean install</span> - only Chart of Accounts and System Users will be backed up. Transactions, customers, vendors, branches, and product data will be <span class="uk-text-bold uk-text-warning">excluded</span> from the backup. Only use this if you are planning to reset the system with a new database.</li>
    </li>
  </ul>
  <li>Click the <span class="uk-text-bold">Backup now</span> button</li>
</ol>
<p>The backup will take a while to finish. After that, a <code>.mysql.gz</code> file with a predefined filename will be saved to your preferred location.</p>

<h3>Backup filename explained:</h3>
Sample filename: <code>uct-alldata-2018-01-28T10-45-15.mysql.gz</code>
<ul>
  <li>"<span class="uk-text-bold">uct-alldata</span>" - this signifies the type of backup used.</li>
  <ul>
    <li>uct-alldata - means "Everything in the database" was selected when the backup was created</li>
    <li>uct-notrans - means "Everything in the database EXCEPT Transactions" was selected when the backup was created</li>
    <li>uct-clean - means "Clean Install - only Chart of Accounts and system users will be backed up" was selected when the backup was created</li>
  </ul>
  <li>"<span class="uk-text-bold">2018-01-28</span>" - signifies the date when the backup was made (Year-Month-Date).</li>
  <li>"<span class="uk-text-bold">T10-45-15</span>" - signifies the time when the backup was made (Hour-Minute-Second).</li>
</ul>
<div class="uk-alert uk-alert-success">
  <span class="uk-text-bold">TIP:</span>
  Backups for the entire database are automatically created periodically.
  <div>Check <code>c:/uct.ais.backup/scheduled</code> or <code>d:/uct.ais.backup/scheduled</code> the most recent backups.</div>
</div>