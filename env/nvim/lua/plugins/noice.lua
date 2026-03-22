return {
	"folke/noice.nvim",
	event = "VeryLazy",
	opts = {
		-- add any options here
		routes = {
			{
				view = "notify",
				filter = { event = "msg_showmode" },
			},
		},
	},
	dependencies = {
		"MunifTanjim/nui.nvim",
		"rcarriga/nvim-notify",
	},
}

