#!/usr/bin/perl
use lib "/usr/lib/xlyrics";
# parse arguments
@ARGV < 1 && print("Usage: downloadlyrics.pl music [download_dir] [more_lyrics?] [list]\n") && exit(-1);
$music = $ARGV[0];
if(@ARGV > 1){
	$lyrics_file = $ARGV[1];
}else{
	$lyrics_file = "$ARGV[0].lrc";
}

use HTTP::Lite;
$http = new HTTP::Lite;

# search lyrics in basic database
%vars = (
	"souqu" => "歌名",
	"souci" => $music,
	"ku" => "db",
	"B1" => "提交"
);
$http->prepare_post(\%vars);
$http->request("http://www.hjqing.com/lrc/index.asp")
	or die "Unable to get document: $!";
if($http->body() =~ /没有你想找的歌词/){
# try the addtion database
	glob $http = new HTTP::Lite;
	%vars = (
		"souqu" => "歌名",
		"souci" => $music,
		"ku" => "dbadd",
		"B1" => "提交"
	);
	$http->prepare_post(\%vars);
	$http->request("http://www.hjqing.com/lrc/index.asp")
		or die "Unable to get documents $!";
	$http->body() =~ /没有你想找的歌词/ && die("find nothing");
}

@lines = split(/\n/, $http->body());
open(INFOFILE, ">/tmp/$music.lrc.info")
	or die "Can not open info file to wirte";
open(OUTFILE, ">$lyrics_file")
	or die "Can not open lyrics file to write";
$count = 0;
for($index = 0; $index < @lines; $index ++){
	if($lines[$index] =~ /^\[.*$/){
		# write the lyrics content to the file
		$lines[$index] =~ s/<\/textarea>//;
		print OUTFILE ("$lines[$index]\n");
	}elsif($lines[$index] =~ /进一步搜索/){
		$index ++;
		for(; $index < @lines; $index ++){
			# add more infomation
			if($lines[$index] =~ /<\/textarea>/){
				last;
			}else{
				print INFOFILE ("$lines[$index]\n");
				$count ++;
			}
		}

		# just get lyrics list or first download and more than one file found
		if(@ARGV == 3
			&& ($ARGV[2] =~ "list" || ($ARGV[2] =~ "first" && $count > 1))){
			close(OUTFILE);
			unlink($lyrics_file);
			exit;
		}

	}
}

close(OUTFILE);
close(INFOFILE);
