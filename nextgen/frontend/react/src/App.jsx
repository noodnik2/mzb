import { useEffect, useState } from "react";

const API_URL = "http://127.0.0.1:8000/api";

function App() {
    const [files, setFiles] = useState([]);
    const [selectedFile, setSelectedFile] = useState(null);
    const [metadata, setMetadata] = useState({});

    useEffect(() => {
        fetch(`${API_URL}/midi-files`)
            .then((res) => res.json())
            .then(setFiles);
    }, []);

    const selectFile = (file) => {
        setSelectedFile(file);
        setMetadata(file.metadata);
    };

    const updateMetadata = () => {
        fetch(`${API_URL}/midi-files/${selectedFile.id}`, {
            method: "PATCH",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify(metadata),
        })
            .then((res) => res.json())
            .then(updatedFile => {
                setFiles(files.map(f => (f.id === updatedFile.id ? updatedFile : f)));
                setSelectedFile(updatedFile);
            });
    };

    return (
        <div className="p-4">
            <h1 className="text-xl font-bold">MIDI Organizer</h1>
            <div className="flex">
                {/* File List */}
                <div className="w-1/3">
                    <h2 className="text-lg">Files</h2>
                    <ul>
                        {files.map(file => (
                            <li key={file.id} className="cursor-pointer" onClick={() => selectFile(file)}>
                                {file.name}
                            </li>
                        ))}
                    </ul>
                </div>

                {/* Metadata Panel */}
                <div className="w-2/3 p-4">
                    {selectedFile ? (
                        <div>
                            <h2 className="text-lg">{selectedFile.name}</h2>
                            <label>Genre:</label>
                            <input
                                type="text"
                                value={metadata.genre || ""}
                                onChange={(e) => setMetadata({ ...metadata, genre: e.target.value })}
                                className="border p-1"
                            />
                            <label>Rating:</label>
                            <input
                                type="number"
                                value={metadata.rating || 0}
                                onChange={(e) => setMetadata({ ...metadata, rating: Number(e.target.value) })}
                                className="border p-1"
                            />
                            <button onClick={updateMetadata} className="bg-blue-500 text-white px-2 py-1 mt-2">
                                Save
                            </button>
                        </div>
                    ) : (
                        <p>Select a file</p>
                    )}
                </div>
            </div>
        </div>
    );
}

export default App;
