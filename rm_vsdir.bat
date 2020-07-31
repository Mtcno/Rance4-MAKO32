@echo off
if Exist .vs (
	rd .vs /S /Q && echo remove .vs
)
pause