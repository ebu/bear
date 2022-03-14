{ ear, bear, dockerTools }:
dockerTools.buildImage {
  name = "bear";
  tag = "latest";
  created = "now";
  contents = [ ear bear ];

  config.Cmd = [ "bin/bear-render" ];
}
