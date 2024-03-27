import ghpages from "gh-pages";


ghpages.publish("build", { nojekyll: true }, 
() => console.log("Deployed to Github Pages!"));
