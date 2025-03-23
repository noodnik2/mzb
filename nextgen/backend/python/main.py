from fastapi import FastAPI, HTTPException
from fastapi.middleware.cors import CORSMiddleware
import os, json, uuid

# Config
MIDI_DIR = "../midi_files"
DB_FILE = "midi_metadata.json"

app = FastAPI()

# Enable CORS for frontend access
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# Load metadata DB
def load_metadata():
    if os.path.exists(DB_FILE):
        with open(DB_FILE, "r") as f:
            return json.load(f)
    return {"files": []}

def save_metadata(data):
    with open(DB_FILE, "w") as f:
        json.dump(data, f, indent=4)

# Scan for MIDI files
def scan_midi_files():
    metadata = load_metadata()
    existing_paths = {file["path"] for file in metadata["files"]}

    for root, _, files in os.walk(MIDI_DIR):
        for file in files:
            if file.endswith(".mid"):
                path = os.path.join(root, file)
                if path not in existing_paths:
                    metadata["files"].append({
                        "id": str(uuid.uuid4()),
                        "name": file,
                        "path": path,
                        "metadata": {"genre": "", "rating": 0, "tags": []}
                    })
    save_metadata(metadata)

@app.get("/api/midi-files")
def get_midi_files():
    return load_metadata()["files"]

@app.get("/api/midi-files/{file_id}")
def get_midi_file(file_id: str):
    files = load_metadata()["files"]
    for file in files:
        if file["id"] == file_id:
            return file
    raise HTTPException(status_code=404, detail="File not found")

@app.patch("/api/midi-files/{file_id}")
def update_midi_metadata(file_id: str, metadata: dict):
    data = load_metadata()
    for file in data["files"]:
        if file["id"] == file_id:
            file["metadata"].update(metadata)
            save_metadata(data)
            return file
    raise HTTPException(status_code=404, detail="File not found")

@app.post("/api/scan")
def trigger_scan():
    scan_midi_files()
    return {"message": "Scan complete"}
