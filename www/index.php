<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN" "http://www.w3.org/TR/html4/strict.dtd">
<html>
<head>
	<title>Wake</title>

	<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
	<link href="layout.css" rel="stylesheet" type="text/css">

	<script type="text/javascript" src="menu.js"></script>
<style type="text/css">
<!--
.copyright {	font-family: fantasy;
	font-size: 9px;
	font-style: italic;
	color: #FFF;
}
.copyright1 {font-family: fantasy;
	font-size: 9px;
	font-style: italic;
}
-->
</style>

<div id="dhtmlgoodies_slidedown_menu">
	<ul>
	<li><a href="index.html">Home</a></li>
	<li><a href="started.html">Getting Started</a></li>
	<li><a href="#">World</a>
		<ul>
		<li><a href="world/races/index.html">Races</a></li>
		<li><a href="world/houses/index.html">Houses</a></li>
		<li><a href="/world/history/index.html">History</a></li>
		<li><a href="world/religion/index.html">Religion</a></li>
		<li><a href="world/maps/index.html">Maps</a></li>
		<li><a href="world/gallery.html">Gallery</a></li>
		</ul>		
    	</li>
	<li><a href="#">Game</a>
		<ul>
		<li><a href="game/rules.html">Rules</a></li>
		<li><a href="game/classes/index.html">Classes</a></li>
           	<li><a href="game/features.html">Features</a></li>
		<li><a href="game/faq.html">FAQ</a></li>
		</ul>
	</li>
	<li><a href="#">Community</a>
		<ul>
		<li><a href="community/players.html">Players</a></li>
		<li><a href="community/staff/index.html">Staff</a></li>
		<li><a href="community/links.html">Links</a></li>
		</ul>
	</li>
      	<li><a href="portal/index.html">Web Portal</a></li>
	<li class="host">Host: wake.codehallow.com</li>
	<li class="host">Port:2177</li>
	<li><a href="http://www.mudconnect.com/cgi-bin/vote_rank.cgi?mud=Wake" target="new"><img src="http://www.mudconnect.com/cgi-bin/get_rank_button.cgi?mud=Wake"alt="Vote for Our Mud on TMC!" border="0" align="left"></a><a href="http://www.topmudsites.com/vote-wake.html"><img src="images/tms.gif" alt="vote" width="108" height="36" border="0"></a></li>
  	</ul>
</div>
<!-- END OF MENU -->
 <?php 
  mysql_connect("localhost","root","password") or die('Could not connect: ' . mysql_error()); 
  mysql_select_db("wake") or die('Could not select db: ' . mysql_error()); 
 ?>
</head>
<body>


<div id="content">

  <p class="date">Welcome to Wake<span class="content"> <strong>Host:</strong> wakemud.com <strong>Port:</strong>2177</span></p>
  <p class="content">
   Be you a fresh faced newcomer or a seasoned veteran, this site is   here to provide you with information and a social environment that enriches your   experience on our mud. If you are new to Wake, or mudding in general, click   "Getting Started" on the left navigation menu to reach a page that will help orient you so you can get started   enjoying our world. If you have any questions, feel free to ask us, either by   email (listed in the Staff section of the site),  using the mud's message   boards, or by leaving a thread on our upcoming forums.<br>
  </p>
  <table width="100%" border="0">
    <tr>
      <td><p class="date">Recent Changes<span class="content"></span></p>
<table border=1>
         <?php
         $query = 'SELECT * FROM changelog ORDER BY timestamp DESC,name DESC';
         $result = mysql_query($query);
         $lastTime = 0;
         while($row = mysql_fetch_assoc($result))
         { ?>
         <tr><td><p><font color=#555555><?= $row['timestamp'];?></font> <font color=#999999> by <?= $row['name']; ?></font></p>
         <p class="content"><font color="white"><?= $row['msg']; ?></font></p></td></tr>
         <?php
         }
         ?>
</table>
      </td>
   </tr>
  </table>
  <p class="content">&nbsp;</p>
  <p class="content">&nbsp;</p>
  <table width="100%" border="0">
    <tr>
      <th scope="col">&nbsp;</th>
      <th class="copyright1" scope="col"><span class="copyright"><img src="images/Wakemini.jpg" width="150" height="94" alt="Wake" /><br />
        All design, content and images are copyright.<br />
        &copy;2010 Wakemud.com</span><br />
      </th>
      <th scope="col">&nbsp;</th>
    </tr>
  </table>
  <p class="content">&nbsp;</p>
      <p class="content">&nbsp;</p>
</div>

	<script type="text/javascript">initSlideDownMenu();</script>

</body>
</html>
