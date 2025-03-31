import React, { useEffect, useState } from "react";

const API_URL = "http://localhost:8000/api/midi-files";

function App() {
    const [midiFiles, setMidiFiles] = useState([]);
    const [loading, setLoading] = useState(true);

    console.log(`app`);

    // Fetch data from backend
    useEffect(() => {
        console.log(`fetching({API_URL}`);
        fetch(API_URL)
            .then((res) => res.json())
            .then((data) => {
                setMidiFiles(data);
                setLoading(false);
            })
            .catch((err) => console.error("Error fetching MIDI files:", err));
    }, []);

    // Update metadata function
    const updateMetadata = (id, field, value) => {
        console.log(`updateMetadata`);
        // Optimistically update UI
        const updatedFiles = midiFiles.map((file) =>
            file.id === id ? { ...file, metadata: { ...file.metadata, [field]: value } } : file
        );
        setMidiFiles(updatedFiles);

        // Send update request to the backend
        fetch(`${API_URL}/${id}`, {
            method: "PATCH",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify({ [field]: value }),
        })
            .then((res) => {
                console.info("Updated metadata");
                if (!res.ok) throw new Error("Failed to update");
                return res.json();
            })
            .catch((err) => {
                console.error("Failed to update metadata:", err);
                // Optionally: revert changes if the request fails
            });
    };

    if (loading) {
        return <h1>Loading MIDI files...</h1>;
    }

    return (
        <div>
            <h1>MIDI Organizer</h1>
            <table border="1">
                <thead>
                <tr>
                    <th>Name</th>
                    <th>Path</th>
                    <th>Rating</th>
                    <th>Genre</th>
                </tr>
                </thead>
                <tbody>
                {midiFiles.map((file) => (
                    <tr key={file.id}>
                        <td>{file.name}</td>
                        <td>{file.path}</td>
                        <td>
                            <input
                                type="number"
                                value={file.metadata.rating || ""}
                                onChange={(e) => {
                                    updateMetadata(file.id, "rating", e.target.value);
                                }}
                            />
                        </td>
                        <td>
                            <input
                                type="text"
                                value={file.metadata.genre || ""}
                                onChange={(e) => {
                                    console.log("Genre changed:", e.target.value);
                                    updateMetadata(file.id, "genre", e.target.value);
                                }}
                            />
                        </td>
                    </tr>
                ))}
                </tbody>
            </table>
        </div>
    );
}

export default App;
