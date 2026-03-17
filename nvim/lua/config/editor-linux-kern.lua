-- configuration for the Linux Kernel coding style
-- -------------------------------------------------------------------
-- Tabs / Indentation ------------------------------------------------
-- The kernel uses tabs that are 8 characters wide, and indentations are also 8 characters
-- (kernel style: tabs are 8 characters)
vim.opt.tabstop      = 8          -- width of a <Tab> character
vim.opt.shiftwidth   = 8          -- number of columns to use for each step of indent
vim.opt.softtabstop  = 8          -- <Tab> counts as 8 spaces in Insert mode
vim.opt.expandtab    = false      -- use real tab characters, no spaces for indent

-- Line‑length limit ------------------------------------------------
-- The kernel style recommends a maximum line length of 80 columns
-- (kernel style: preferred limit 80 columns)
vim.opt.textwidth    = 80         -- maximum length for automatic wrapping
vim.opt.colorcolumn  = "80"       -- highlight column 80 for visual reference

