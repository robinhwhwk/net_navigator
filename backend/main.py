import uvicorn

if __name__ == "__main__":
    # run a Uvicorn server on port 8000 and reload on every file change
    uvicorn.run("app.api:app", host="0.0.0.0", port=8000, reload=True)