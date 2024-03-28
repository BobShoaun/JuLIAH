import adapter from "@sveltejs/adapter-static";
import { vitePreprocess } from "@sveltejs/vite-plugin-svelte";
/** @type {import('@sveltejs/kit').Config} */
const config = {
  kit: {
    adapter: adapter({
      fallback: "404.html",
    }),
    //   {
    //   pages: "build",
    //   assets: "build",
    //   fallback: "404.html",
    //   precompress: false,
    //   strict: true,
    // }
    paths: {
      base: process.env.NODE_ENV === "production" ? "/JuLIAH" : "",
    },
  },
  preprocess: vitePreprocess(),
};
export default config;
