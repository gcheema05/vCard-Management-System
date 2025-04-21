<h1>📇 vCard Management System</h1>

<p>
A cross-platform vCard contact management application built using <strong>C</strong>, <strong>Python</strong>, and <strong>MySQL</strong>. This system includes a C-based parsing and writing library for vCard files (RFC 6350), a Python terminal-based interface built with <strong>Asciimatics</strong>, and a relational database backend for persistent storage and querying of contact data. It is designed for managing contact lists locally and interactively.
</p>

<hr />

<h2>✅ Features</h2>

<ul>
  <li>📂 Parse and validate vCard (.vcf) files using a C shared library</li>
  <li>✏️ View, edit, and create contact entries through a terminal UI</li>
  <li>🧩 Python frontend integrated with C backend using <code>ctypes</code></li>
  <li>🗃️ Store and manage contact and file data in a MySQL database</li>
  <li>🔍 Execute database queries to display and filter contacts</li>
</ul>

<hr />

<h2>🛠 Tech Stack</h2>

<table>
  <tr><td><strong>Languages</strong></td><td>C, Python, SQL</td></tr>
  <tr><td><strong>Frontend</strong></td><td>Python (Asciimatics)</td></tr>
  <tr><td><strong>Backend</strong></td><td>C (Shared Library), MySQL</td></tr>
  <tr><td><strong>Integration</strong></td><td>Ctypes</td></tr>
  <tr><td><strong>Build Tools</strong></td><td>Makefile</td></tr>
</table>

<hr />

<h2>📁 Project Structure</h2>

<pre><code>
vCard-Management-System/
├── Makefile                 # Builds the C shared library (libvcparser.so)
├── bin/
│   ├── libvcparser.so       # Compiled C shared library
│   ├── A3main.py            # Python UI entry point
│   └── cards/               # Folder containing .vcf files
├── src/                     # All C source files (VCParser.c, VCHelpers.c, LinkedListAPI.c, etc.)
├── include/                 # Header files (VCHelpers.h, etc.)
</code></pre>


<hr />

<h2>📥 How to Download and Run</h2>

<h4>🔧 Requirements</h4>
<ul>
  <li>GCC (C99), Make, Python 3</li>
  <li>MySQL client library (MySQL Connector)</li>
  <li>Linux, macOS, or WSL (Windows Subsystem for Linux)</li>
</ul>

<h4>📦 Clone the Project</h4>

<pre><code>
git clone https://github.com/gcheema05/vCard-Management-System.git
cd vCard-Management-System
</code></pre>

<h4>🔨 Build the C Shared Library</h4>

<pre><code>
make parser
</code></pre>

<h4>🚀 Launch the Interface</h4>

<pre><code>
cd bin
python3 A3main.py
</code></pre>


<h2>📇 Application Workflow</h2>

<h3>Main View (vCard List)</h3>
<ul>
  <li>Lists all valid vCard files from the <code>cards/</code> folder</li>
  <li>Files are validated using the C library before being shown</li>
  <li>Supports editing and creating new vCard files</li>
</ul>

<h3>vCard Details View</h3>
<ul>
  <li>Displays fields such as contact name (FN), birthday, anniversary</li>
  <li>Allows user to modify contact name and save changes</li>
  <li>Uses C backend for writing and validation</li>
</ul>

<h3>Login View</h3>
<ul>
  <li>Prompts for username, password, and database name</li>
  <li>Establishes connection to remote MySQL server</li>
  <li>Tables are created if they don’t already exist</li>
</ul>

<h3>Database View</h3>
<ul>
  <li><strong>Display All Contacts:</strong> Lists all contacts with file name</li>
  <li><strong>Find Contacts Born in June:</strong> Filters contacts with June birthdays sorted by age</li>
  <li>Displays current DB row counts after each query</li>
</ul>

<hr />

<h2>🗃️ MySQL Schema</h2>

<h3>📄 FILE Table</h3>
<ul>
  <li><code>file_id</code> – Primary Key</li>
  <li><code>file_name</code>, <code>last_modified</code>, <code>creation_time</code></li>
</ul>

<h3>👤 CONTACT Table</h3>
<ul>
  <li><code>contact_id</code> – Primary Key</li>
  <li><code>name</code>, <code>birthday</code>, <code>anniversary</code></li>
  <li><code>file_id</code> – Foreign Key referencing FILE(file_id)</li>
</ul>

<p><strong>Foreign key constraints</strong> ensure cascading deletes for synchronized cleanup.</p>

<hr />

<h2>💡 Highlights</h2>

<ul>
  <li>Line-unfolding logic implemented in parser</li>
  <li>Handles both structured and compound property formats</li>
  <li>Interactive UI with tab and arrow-key navigation</li>
  <li>Fully integrated validation and write flow between frontend and backend</li>
</ul>

<hr />

<h2>🚀 Future Enhancements</h2>

<ul>
  <li>📤 Export contacts as JSON or CSV</li>
  <li>📆 UI editing for additional properties like birthday & anniversary</li>
  <li>🔍 Advanced database query filters (e.g. partial name search)</li>
  <li>🧪 Add unit tests for shared library functions</li>
  <li>🌐 Web-based GUI version using Flask or React</li>
</ul>
