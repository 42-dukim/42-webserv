// 메뉴 전환
const menuItems = document.querySelectorAll('.menu-item');
const contentSections = document.querySelectorAll('.content-section');
const headerTitle = document.getElementById('headerTitle');

menuItems.forEach(item => {
	item.addEventListener('click', () => {
		const section = item.dataset.section;

		menuItems.forEach(m => m.classList.remove('active'));
		item.classList.add('active');

		contentSections.forEach(s => s.classList.remove('active'));
		document.getElementById(section).classList.add('active');

		const titles = {
			'dashboard': '홈',
			'upload': '업로드',
			'files': '내 파일'
		};
		headerTitle.textContent = titles[section];
	});
});

// 현재 시간 표시
function updateTime() {
	const now = new Date();
	const options = {
		year: 'numeric',
		month: 'long',
		day: 'numeric',
		hour: '2-digit',
		minute: '2-digit'
	};
	document.getElementById('currentTime').textContent =
		now.toLocaleDateString('ko-KR', options);
}
updateTime();
setInterval(updateTime, 60000);

// 파일 업로드 기능
const uploadArea = document.getElementById('uploadArea');
const fileInput = document.getElementById('fileInput');
const fileList = document.getElementById('fileList');
const uploadBtn = document.getElementById('uploadBtn');
const successMessage = document.getElementById('successMessage');

let files = [];

// 페이지 로드 시 파일 목록 가져오기
document.addEventListener('DOMContentLoaded', () => {
	loadFileList();
	// 5초마다 파일 목록 갱신
	// setInterval(loadFileList, 5000);
});

// 렌더링 헬퍼: 서버가 보낸 HTML로 문서 전체를 교체
function renderErrorHtml(html) {
    try {
        document.open();
        document.write(html);
        document.close();
    } catch (e) {
        console.error('HTML 렌더링 실패:', e);
        showNotification('❌ 에러 페이지 렌더링 실패', 'error');
    }
}

// 알림 표시 함수
function showNotification(message, type = 'success') {
    const notification = document.createElement('div');
    notification.className = 'notification';
    notification.style.cssText = `
        position: fixed;
        top: 20px;
        right: 20px;
        padding: 12px 18px;
        background: ${type === 'success' ? '#2ecc71' : '#e74c3c'};
        color: white;
        border-radius: 8px;
        box-shadow: 0 5px 15px rgba(0,0,0,0.12);
        z-index: 10000;
        font-weight: 600;
    `;
    notification.textContent = message;
    document.body.appendChild(notification);

    setTimeout(() => {
        notification.style.opacity = '0';
        notification.style.transition = 'opacity 0.3s ease';
        setTimeout(() => notification.remove(), 300);
    }, 3000);
}

// 유틸: HTML 이스케이프
function escapeHtml(str) {
    if (!str) return '';
    return str.replace(/[&<>"']/g, function (m) {
        return ({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'})[m];
    });
}

// 파일 리스트 로드
async function loadFileList() {
    try {
        const response = await fetch('/cgi-bin/list_files.py');
        const contentType = response.headers.get('content-type') || '';
		console.log(response)
        // 서버가 에러 페이지(HTML)를 직접 반환한 경우 전체 문서로 렌더링
        if (!response.ok || !contentType.includes('application/json')) {
            const html = await response.text();
            renderErrorHtml(html);
            return;
        }

        const data = await response.json();
        if (data && data.success) {
            updateDashboard({
                totalFiles: data.total_count,
                totalSize: data.total_size,
                todayUploads: calculateTodayUploads(data.files)
            });
            updateFilesList(data.files);
            updateRecentActivity(data.files);
        } else {
            console.error('파일 리스트 로드 실패 (API):', data && data.error);
            showNotification('❌ 파일 리스트 로드 실패: ' + (data && data.error ? data.error : '알 수 없는 오류'), 'error');
        }
    } catch (err) {
        console.error('파일 리스트 로드 오류:', err);
        showNotification('❌ 파일 리스트 로드 실패', 'error');
    }
}

// 오늘 업로드된 파일 수 계산
function calculateTodayUploads(files) {
	const now = Date.now() / 1000;
	const oneDayAgo = now - 86400;
	return files.filter(f => f.modified > oneDayAgo).length;
}

// 대시보드 통계 업데이트
function updateDashboard(stats) {
	document.getElementById('totalFiles').textContent = stats.totalFiles;
	document.getElementById('totalStorage').textContent = formatFileSize(stats.totalSize);
	document.getElementById('todayUploads').textContent = stats.todayUploads;

	// 저장 공간 사용률 업데이트 (최대 10MB)
	const maxStorage = 10 * 1024 * 1024; // 10MB
	let usagePercent = Math.round((stats.totalSize / maxStorage) * 100 * 10) / 10; // 소수점 첫째자리까지

	// 최소 1% 표시 (파일이 있으면 최소한 보이게)
	if (stats.totalSize > 0 && usagePercent < 1) {
		usagePercent = 1;
	}
	usagePercent = Math.min(usagePercent, 100);

	const progressBars = document.querySelectorAll('.progress-bar');
	if (progressBars.length > 0) {
		const progressFill = progressBars[0].querySelector('.progress-fill');
		if (progressFill) {
			progressFill.style.width = usagePercent + '%';
			progressFill.textContent = '';
		}
	}
}

// 파일 리스트 업데이트
function updateFilesList(files) {
	const container = document.getElementById('filesListContainer');

	if (!files || files.length === 0) {
		container.innerHTML = '<div style="text-align: center; padding: 40px; color: #666;">📭 업로드된 파일이 없습니다</div>';
		return;
	}

	// 수정 시간 기준 내림차순 정렬 (최신 파일이 위로)
	files.sort((a, b) => b.modified - a.modified);

	let html = '<div class="files-grid">';

	files.forEach(file => {
		const icon = getFileIcon(file.name);
		const sizeStr = formatFileSize(file.size);

		html += `
            <div class="file-card">
                <div class="file-icon">${icon}</div>
                <div class="file-info">
                    <div class="file-name" title="${file.name}">${file.name}</div>
                    <div class="file-meta">
                        <span>${sizeStr}</span>
                        <span>${file.modified_str}</span>
                    </div>
                </div>
                <button class="file-delete-btn" onclick="deleteFile('${file.name}')">🗑️</button>
            </div>
        `;
	});

	html += '</div>';
	container.innerHTML = html;
}

// 최근 활동 업데이트
function updateRecentActivity(files) {
	const container = document.getElementById('recentActivityContainer');

	if (!files || files.length === 0) {
		container.innerHTML = '<div class="activity-item"><div class="activity-text">📭 최근 활동이 없습니다</div></div>';
		return;
	}

	// 수정 시간 기준 내림차순 정렬하여 최근 4개만 표시
	const recentFiles = [...files].sort((a, b) => b.modified - a.modified).slice(0, 4);

	let html = '';
	recentFiles.forEach(file => {
		const icon = getFileIcon(file.name);
		const timeAgo = getTimeAgo(file.modified);

		html += `
            <div class="activity-item">
                <div class="activity-time">${timeAgo}</div>
                <div class="activity-text">${icon} ${file.name} 파일이 업로드되었습니다</div>
            </div>
        `;
	});

	container.innerHTML = html;
}

// 상대적 시간 표시 (예: "5분 전", "3시간 전")
function getTimeAgo(timestamp) {
	const now = Date.now() / 1000; // 현재 시간 (초)
	const diff = now - timestamp;

	if (diff < 60) {
		return '방금 전';
	} else if (diff < 3600) {
		const minutes = Math.floor(diff / 60);
		return `${minutes}분 전`;
	} else if (diff < 86400) {
		const hours = Math.floor(diff / 3600);
		return `${hours}시간 전`;
	} else if (diff < 604800) {
		const days = Math.floor(diff / 86400);
		return days === 1 ? '어제' : `${days}일 전`;
	} else {
		const weeks = Math.floor(diff / 604800);
		return `${weeks}주 전`;
	}
}

// 파일 아이콘 가져오기
function getFileIcon(filename) {
	const ext = filename.split('.').pop().toLowerCase();
	const iconMap = {
		'pdf': '📄',
		'doc': '📝', 'docx': '📝',
		'xls': '📊', 'xlsx': '📊',
		'ppt': '📊', 'pptx': '📊',
		'jpg': '🖼️', 'jpeg': '🖼️', 'png': '🖼️', 'gif': '🖼️', 'svg': '🖼️',
		'mp3': '🎵', 'wav': '🎵', 'ogg': '🎵',
		'mp4': '🎬', 'avi': '🎬', 'mkv': '🎬',
		'zip': '📦', 'rar': '📦', 'tar': '📦', 'gz': '📦',
		'txt': '📃',
		'py': '🐍', 'js': '📜', 'html': '🌐', 'css': '🎨',
		'cpp': '⚙️', 'c': '⚙️', 'hpp': '⚙️', 'h': '⚙️'
	};
	return iconMap[ext] || '📄';
}

// 파일 크기 포맷
function formatFileSize(bytes) {
	if (bytes === 0) return '0 Bytes';
	const k = 1024;
	const sizes = ['Bytes', 'KB', 'MB', 'GB'];
	const i = Math.floor(Math.log(bytes) / Math.log(k));
	return Math.round(bytes / Math.pow(k, i) * 100) / 100 + ' ' + sizes[i];
}

// 드래그 앤 드롭 이벤트
uploadArea.addEventListener('click', () => {
	fileInput.click();
});

fileInput.addEventListener('change', (e) => {
	handleFiles(e.target.files);
});

uploadArea.addEventListener('dragover', (e) => {
	e.preventDefault();
	uploadArea.classList.add('dragover');
});

uploadArea.addEventListener('dragleave', () => {
	uploadArea.classList.remove('dragover');
});

uploadArea.addEventListener('drop', (e) => {
	e.preventDefault();
	uploadArea.classList.remove('dragover');
	handleFiles(e.dataTransfer.files);
});

function handleFiles(newFiles) {
	files = [...files, ...Array.from(newFiles)];
	displayFiles();
	uploadBtn.disabled = files.length === 0;
}

function displayFiles() {
	fileList.innerHTML = '';
	files.forEach((file, idx) => {
		const fileItem = document.createElement('div');
		fileItem.className = 'file-item';
		fileItem.innerHTML = `
            <div class="file-info">
                <div class="file-name">${file.name}</div>
                <div class="file-size">${formatFileSize(file.size)}</div>
            </div>
            <button class="delete-btn" onclick="removeFile(${idx})">삭제</button>
        `;
		fileList.appendChild(fileItem);
	});
}

function removeFile(idx) {
	files.splice(idx, 1);
	displayFiles();
	uploadBtn.disabled = files.length === 0;
}

uploadBtn.addEventListener('click', async () => {
	if (files.length === 0) return;

	const formData = new FormData();
	files.forEach((file) => {
		formData.append('file', file);
	});

	try {
		uploadBtn.disabled = true;
		uploadBtn.textContent = '업로드 중...';

		const response = await fetch('/cgi-bin/upload.py', {
			method: 'POST',
			body: formData
		});

		if (response.ok) {
			// 파일 목록 새로고침
			loadFileList();

			successMessage.innerHTML = `
                <div class="success-message">
                    ✅ ${files.length}개의 파일이 성공적으로 업로드되었습니다!
                </div>
            `;

			setTimeout(() => {
				files = [];
				displayFiles();
				uploadBtn.disabled = true;
				uploadBtn.textContent = '업로드';
				fileInput.value = '';
				successMessage.innerHTML = '';
			}, 3000);
		} else {
			throw new Error('업로드 실패');
		}
	} catch (error) {
		successMessage.innerHTML = `
            <div class="success-message" style="background: #e74c3c;">
                ❌ 업로드 중 오류가 발생했습니다: ${error.message}
            </div>
        `;
		uploadBtn.disabled = false;
		uploadBtn.textContent = '업로드';
	}
});

// 토글 스위치
function toggleSwitch(element) {
	element.classList.toggle('active');
}

// 홈으로 이동
function goToHome() {
	// 대시보드 메뉴 활성화
	menuItems.forEach(m => m.classList.remove('active'));
	menuItems[0].classList.add('active');

	// 대시보드 섹션 표시
	contentSections.forEach(s => s.classList.remove('active'));
	document.getElementById('dashboard').classList.add('active');

	// 헤더 타이틀 변경
	headerTitle.textContent = '홈';
}

// 파일 삭제 함수
async function deleteFile(filename) {
	if (!confirm(`"${filename}" 파일을 삭제하시겠습니까?`)) {
		return;
	}

	try {
		const response = await fetch(`/cgi-bin/delete_file.py?filename=${filename}`, {
			method: 'DELETE',
			//headers: {
			//    'Content-Type': 'application/json'
			//},
			//body: JSON.stringify({ filename: filename })
		});

		const result = await response.json();

		if (result.success) {
			// 파일 목록 새로고침
			loadFileList();

			// 성공 메시지 표시 (임시)
			showNotification('✅ 파일이 삭제되었습니다', 'success');
		} else {
			showNotification('❌ 삭제 실패: ' + result.error, 'error');
		}
	} catch (error) {
		console.error('파일 삭제 오류:', error);
		showNotification('❌ 삭제 중 오류가 발생했습니다', 'error');
	}
}

// 알림 표시 함수
function showNotification(message, type = 'success') {
	const notification = document.createElement('div');
	notification.className = 'notification';
	notification.style.cssText = `
        position: fixed;
        top: 20px;
        right: 20px;
        padding: 15px 25px;
        background: ${type === 'success' ? '#2ecc71' : '#e74c3c'};
        color: white;
        border-radius: 10px;
        box-shadow: 0 5px 15px rgba(0,0,0,0.2);
        z-index: 1000;
        animation: slideIn 0.3s ease;
    `;
	notification.textContent = message;
	document.body.appendChild(notification);

	setTimeout(() => {
		notification.style.animation = 'slideOut 0.3s ease';
		setTimeout(() => notification.remove(), 300);
	}, 3000);
}
