"""Tiny local web server for the Food Label Scanner.
It only connects the browser to the C program (scanner / scanner.exe).
Run from the project root:   python server.py     then open http://localhost:8000
Needs Python 3 only (no extra packages)."""
import json, os, subprocess, uuid
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

ROOT = os.path.dirname(os.path.abspath(__file__))
EXE = os.path.join(ROOT, "scanner.exe" if os.name == "nt" else "scanner")
UPLOADS = os.path.join(ROOT, "images", "uploads")
os.makedirs(UPLOADS, exist_ok=True)

def run(args):
    try:
        r = subprocess.run([EXE] + args, cwd=ROOT, capture_output=True, timeout=90,
                           encoding="utf-8", errors="replace")
        return r.stdout or json.dumps({"ok": False, "error": r.stderr or "scanner returned nothing"})
    except FileNotFoundError:
        return json.dumps({"ok": False, "error": "Build the C program first (build.bat or make)."})
    except Exception as e:
        return json.dumps({"ok": False, "error": str(e)})

def num(v):
    try: return str(int(v) & 63)
    except Exception: return "0"

class H(BaseHTTPRequestHandler):
    def send(self, body, ctype="application/json", code=200):
        b = body.encode("utf-8") if isinstance(body, str) else body
        self.send_response(code)
        self.send_header("Content-Type", ctype + "; charset=utf-8" if "text" in ctype or "json" in ctype else ctype)
        self.send_header("Content-Length", str(len(b)))
        self.end_headers()
        self.wfile.write(b)

    def do_GET(self):
        if self.path in ("/", "/index.html"):
            with open(os.path.join(ROOT, "web", "index.html"), "rb") as f:
                self.send(f.read(), "text/html")
        elif self.path == "/api/history":
            self.send(run(["--history-json"]))
        else:
            self.send("not found", "text/plain", 404)

    def do_POST(self):
        data = self.rfile.read(int(self.headers.get("Content-Length", 0)))
        product = (self.headers.get("X-Product") or "Web scan")[:40].strip() or "Web scan"
        if self.path == "/api/analyze":
            j = json.loads(data or b"{}")
            product = (j.get("product") or "Web scan")[:40].strip() or "Web scan"
            self.send(run(["--json", num(j.get("profile", 0)), product, j.get("text", "")]))
        elif self.path == "/api/ocr":
            ext = ".png" if "png" in (self.headers.get("Content-Type") or "") else ".jpg"
            path = os.path.join(UPLOADS, uuid.uuid4().hex[:8] + ext)
            with open(path, "wb") as f: f.write(data)
            try:
                self.send(run(["--json-ocr", num(self.headers.get("X-Profile", 0)), product, path]))
            finally:
                os.remove(path)
        else:
            self.send("not found", "text/plain", 404)

    def log_message(self, *a): pass

if __name__ == "__main__":
    print("Food Label Scanner running at http://localhost:8000  (Ctrl+C to stop)")
    ThreadingHTTPServer(("127.0.0.1", 8000), H).serve_forever()
