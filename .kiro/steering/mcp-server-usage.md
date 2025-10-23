---
inclusion: always
---

# MCP Server Usage Guidelines

## Core Principle

**Always prefer MCP tools over shell commands when functionality overlaps.** Use parallel invocation for independent operations.

## Filesystem MCP (Priority 1)

**CRITICAL: Use Filesystem MCP tools for ALL file and directory operations.**

### Key Tools
- `mcp_filesystem_read_multiple_files` - Read 2+ files simultaneously (preferred over single reads)
- `mcp_filesystem_read_text_file` - Read single file (supports head/tail)
- `mcp_filesystem_edit_file` - Targeted edits with git-style diffs (preferred for modifications)
- `mcp_filesystem_write_file` - Create new files or complete rewrites
- `mcp_filesystem_search_files` - Recursive pattern-based search
- `mcp_filesystem_list_directory` - List contents
- `mcp_filesystem_directory_tree` - Recursive JSON tree view
- `mcp_filesystem_create_directory` - Create directories
- `mcp_filesystem_move_file` - Move or rename
- `mcp_filesystem_get_file_info` - File metadata

### Rules
- Batch file reads using `read_multiple_files`
- Use `edit_file` to show diffs for changes
- Check allowed directories if access fails

## Sequential Thinking MCP

**Use for:** Complex multi-step problems, planning with revision, unclear scope, hypothesis generation/verification

**Tool:** `mcp_sequential_thinking_sequentialthinking`

**Features:** Adjustable thought count, revision capability, branching, backtracking

## Context7 MCP

**Use for:** Library/framework documentation, API references, current package information

**Workflow:**
1. `mcp_Context7_resolve_library_id` with package name (skip if user provides `/org/project` format)
2. `mcp_Context7_get_library_docs` with returned ID
3. Optional: Set `topic` to focus, adjust `tokens` for context size (default: 5000)

## Brave Search MCP

**Use for:** Current information, news, technical research, images/videos, location queries

### Tools by Use Case
- `brave_web_search` - General searches (default)
- `brave_news_search` - Recent events, articles
- `brave_video_search` - Video content
- `brave_image_search` - Images
- `brave_local_search` - Location-based (Pro plan only, fallback to web search)
- `brave_summarizer` - AI summaries (Pro AI plan, requires `summary=true` in web search)

### Best Practices
- Use specific technical terms and service names
- Quote exact phrases for precision
- Always cite sources with markdown hyperlinks
- Verify plan requirements for Pro features

## Error Handling

If MCP tool fails:
1. Verify server configuration in `.kiro/settings/mcp.json`
2. Check server is not disabled
3. Validate parameters
4. For Filesystem MCP, confirm paths are within allowed directories
5. Suggest reconnection from MCP Server view