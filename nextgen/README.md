# 🎵 MIDI File Organizer

A simple tool to organize MIDI files by genre, rating, and tags.

## 🚀 Setup

### Backend 

#### Golang

```shell
$ cd backend/golang
$ go mod init midi-organizer
$ go mod tidy
$ go run main.go
```

#### FastAPI

```shell
$ cd backend/python
$ python -m venv venv
$ source venv/bin/activate  # Windows: venv\Scripts\activate
$ pip install -r requirements.txt
$ uvicorn main:app --reload
```

### Frontend

#### React + Vite

```shell
cd frontend/react
npm install
npm run dev
```