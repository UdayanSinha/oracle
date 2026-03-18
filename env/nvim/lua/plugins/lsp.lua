return {
	{
		-- Mason for managing LSP servers
		"williamboman/mason.nvim",
		opts = {},
	},
	{
		-- Mason LSP client
		"williamboman/mason-lspconfig.nvim",
		opts = {
			ensure_installed = {
				"clangd",     -- for C/C++
				"rust_analyzer", -- for Rust
			},
		},
	},
	{
		-- LSP configuration
		"neovim/nvim-lspconfig",
		config = function()
			local opts = { noremap = true, silent = true }

			-- Key mappings for LSP
			vim.keymap.set("n", "<leader>e", vim.diagnostic.open_float, opts)
			vim.keymap.set("n", "[d", vim.diagnostic.goto_prev, opts)
			vim.keymap.set("n", "]d", vim.diagnostic.goto_next, opts)
			vim.keymap.set("n", "<leader>q", vim.diagnostic.setloclist, opts)

			-- Setup LSP servers using the new approach
			vim.lsp.enable('clangd')
			vim.lsp.enable('rust')
		end,
	},
}

