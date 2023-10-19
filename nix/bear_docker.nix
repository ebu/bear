{ ear, bear, dockerTools }:
dockerTools.buildImage {
  name = "bear";
  tag = "latest";
  created = "now";
  copyToRoot = [ ear bear ];

  config.Cmd = [ "bin/bear-render" ];
}
