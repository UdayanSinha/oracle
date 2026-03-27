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
vim.opt.guicursor       = "i:block"   -- block cursor in insert mode
vim.cmd.filetype("plugin indent on")  -- filetype detection, plugins, and indentation

-- ------------------------------------------------------------------
-- Undo setup
-- ------------------------------------------------------------------
vim.opt.undodir  = os.getenv('HOME') .. '/.nvim/undodir' -- directory for undo files
vim.opt.undofile = true                                  -- persistent undo

-- ------------------------------------------------------------------
-- Whitespace handling
-- ------------------------------------------------------------------
-- enable rendering of whitespace
vim.opt.list        = true
vim.opt.listchars = {
	space   = "·",       -- normal spaces
	tab     = "→ ",      -- tabs
	trail   = "·",       -- trailing spaces
	eol     = "↵",       -- end‑of‑line (CR) marker
}

-- Highlight trailing spaces in red
vim.cmd('match ErrorMsg "\\s\\+$"')   -- highlights any trailing whitespace

-- Remove trailing whitespaces
vim.api.nvim_create_autocmd('FileType', {
  group = vim.api.nvim_create_augroup('trim_whitespaces', { clear = true }),
  desc = 'Trim trailing white spaces',
  pattern = '*',
  callback = function()
    vim.api.nvim_create_autocmd('BufWritePre', {
      pattern = '<buffer>',
      -- Trim trailing whitespaces
      callback = function()
        -- Save cursor position to restore later
        local curpos = vim.api.nvim_win_get_cursor(0)
        -- Search and replace trailing whitespaces
        vim.cmd([[keeppatterns %s/\s\+$//e]])
        vim.api.nvim_win_set_cursor(0, curpos)
      end,
    })
  end,
})
