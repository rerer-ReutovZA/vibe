import { defineConfig, globalIgnores } from "eslint/config";
import nextVitals from "eslint-config-next/core-web-vitals";
import nextTs from "eslint-config-next/typescript";

const eslintConfig = defineConfig([
  ...nextVitals,
  ...nextTs,
  // Override default ignores of eslint-config-next.
  globalIgnores([
    // Default ignores of eslint-config-next:
    ".next/**",
    "out/**",
    "build/**",
    "next-env.d.ts",
    // DLL build directory
    "dll/build/**",
    "dll/out/**",
    "dll/*.dll",
    "dll/*.lib",
    "dll/*.exp",
    "dll/*.obj",
    "dll/*.pdb",
    "dll/*.ilk",
    "dll/*.log",
    "dll/.vs/**",
  ]),
]);

export default eslintConfig;
