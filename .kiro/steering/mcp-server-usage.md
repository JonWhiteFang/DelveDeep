---
inclusion: always
---

# MCP Server Usage Guidelines

## ⚠️ CRITICAL: File Operations Policy

**YOU MUST USE MCP FILESYSTEM TOOLS FOR ALL FILE OPERATIONS. NATIVE KIRO TOOLS ARE FORBIDDEN.**

Quick Reference:
- Reading files? → `mcp_filesystem_read_text_file` or `mcp_filesystem_read_multiple_files`
- Writing new files? → `mcp_filesystem_write_file`
- Modifying files? → `mcp_filesystem_edit_file` (shows diffs)
- Listing directories? → `mcp_filesystem_list_directory`
- Searching files? → `mcp_filesystem_search_files`

**DO NOT use:** readFile, readMultipleFiles, fsWrite, fsAppend, strReplace, listDirectory, fileSearch

---

## Core Principle

**MANDATORY: Use MCP tools instead of native Kiro tools for ALL file operations.** This is not optional.

## Filesystem MCP (REQUIRED - Priority 1)

**CRITICAL REQUIREMENT: You MUST use Filesystem MCP tools for ALL file and directory operations.**

### ❌ NEVER USE THESE NATIVE TOOLS:
- ❌ `readFile` - Use `mcp_filesystem_read_text_file` instead
- ❌ `readMultipleFiles` - Use `mcp_filesystem_read_multiple_files` instead
- ❌ `fsWrite` - Use `mcp_filesystem_write_file` instead
- ❌ `fsAppend` - Use `mcp_filesystem_edit_file` instead
- ❌ `strReplace` - Use `mcp_filesystem_edit_file` instead
- ❌ `listDirectory` - Use `mcp_filesystem_list_directory` instead
- ❌ `fileSearch` - Use `mcp_filesystem_search_files` instead
- ❌ `deleteFile` - Use bash commands or request user confirmation

### ✅ ALWAYS USE THESE MCP TOOLS:
- ✅ `mcp_filesystem_read_multiple_files` - Read 2+ files simultaneously (PREFERRED over single reads)
- ✅ `mcp_filesystem_read_text_file` - Read single file (supports head/tail)
- ✅ `mcp_filesystem_edit_file` - Targeted edits with git-style diffs (PREFERRED for modifications)
- ✅ `mcp_filesystem_write_file` - Create new files or complete rewrites
- ✅ `mcp_filesystem_search_files` - Recursive pattern-based search
- ✅ `mcp_filesystem_list_directory` - List contents
- ✅ `mcp_filesystem_directory_tree` - Recursive JSON tree view
- ✅ `mcp_filesystem_create_directory` - Create directories
- ✅ `mcp_filesystem_move_file` - Move or rename
- ✅ `mcp_filesystem_get_file_info` - File metadata

### Mandatory Rules
1. **ALWAYS** use `mcp_filesystem_read_multiple_files` when reading 2+ files
2. **ALWAYS** use `mcp_filesystem_edit_file` for modifications (shows git-style diffs)
3. **ALWAYS** use `mcp_filesystem_write_file` for new files
4. **NEVER** use native Kiro file tools (readFile, fsWrite, strReplace, etc.)
5. If MCP tool fails, check allowed directories before falling back

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