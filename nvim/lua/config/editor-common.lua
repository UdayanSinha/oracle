-- ------------------------------------------------------------------
-- Miscellaneous
-- ------------------------------------------------------------------
vim.opt.autoindent      = true        -- keep indentation from previous line
vim.opt.smartindent     = false       -- don't auto‑indent based on syntax
vim.opt.number          = true        -- absolute line numbers
vim.opt.relativenumber  = false       -- relative line numbers
vim.opt.cursorline      = true        -- highlight the current line
vim.opt.wrap            = false       -- disable line wrapping
vim.opt.signcolumn      = "yes:1"     -- show sign column
vim.opt.scrolloff       = 8           -- minimum lines to keep above & below cursor
vim.opt.inccommand      = "nosplit"   -- shows the effects of a command incrementally in the buffer
vim.cmd.filetype("plugin indent on")  -- filetype detection, plugins, and indentation

-- ------------------------------------------------------------------
-- Undo setup
-- ------------------------------------------------------------------
vim.opt.undodir  = os.getenv('HOME') .. '/.vim/undodir' -- directory for undo files
vim.opt.undofile = true                                 -- persistent undo

-- ------------------------------------------------------------------
-- Render all whitespace characters
-- ------------------------------------------------------------------
vim.opt.list        = true            -- enable rendering of whitespace
vim.opt.listchars = {
	space   = "·",   -- show a visible symbol for normal spaces
	tab     = "→ ",  -- show a tab as “→ ” (you can change the symbol)
	trail   = "·",   -- show trailing spaces
	nbsp    = "␣",   -- non‑breaking spaces
	eol     = "↵",   -- show end‑of‑line marker
	extends = "❯",   -- show when a line is too long
	precedes= "❮",   -- show when a line starts before the viewport
}
vim.opt.showbreak = "↪ "              -- optional: visual marker for wrapped lines

-- Highlight trailing spaces in red
vim.cmd('match ErrorMsg "\\s\\+$"')   -- highlights any trailing whitespace

